package main

import (
	"database/sql"
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"math"
	"os"
	"os/exec"
	"path/filepath"
	"strconv"
	"strings"
	"text/template"

	"code.google.com/p/go-uuid/uuid"
	_ "github.com/mxk/go-sqlite/sqlite3"
	"github.com/rwcarlsen/cyan/nuc"
	"github.com/rwcarlsen/cyan/post"
	"github.com/rwcarlsen/cyan/query"
)

var scenfile = flag.String("scen", "scenario.json", "file containing problem scenification")
var dakotaGen = flag.Bool("init", false, "true to generate the dakota input file")
var dakotaif = flag.String("o", "", "name of generated dakota input file")
var metric = flag.String("metric", "", "path to cyclus db to calc objective for")
var sweep = flag.String("sweep", "", "path to output db file")

const tmpDir = "cyctmp"

func main() {
	flag.Parse()
	log.SetFlags(log.Lshortfile)

	// load problem scen file
	scen := &Scenario{}
	err := scen.Load(*scenfile)
	fatalif(err)

	// build dakota input file from template and return
	if *dakotaGen {
		if *dakotaif == "" {
			*dakotaif = filepath.Base(scen.DakotaTmpl) + ".gen"
		}

		t := template.Must(template.ParseFiles(scen.DakotaTmpl))
		f, err := os.Create(*dakotaif)
		fatalif(err)
		defer f.Close()

		err = t.Execute(f, scen)
		fatalif(err)
		return
	}

	if *sweep != "" {
		db, err := sql.Open("sqlite3", *sweep)
		fatalif(err)
		defer db.Close()
		db.Exec(`
			CREATE TABLE IF NOT EXISTS Sweep (
				f1_t1 INTEGER,
				f1_t2 INTEGER,
				f1_t3 INTEGER,
				f1_t4 INTEGER,
				f1_t5 INTEGER,
				f2_t1 INTEGER,
				f2_t2 INTEGER,
				f2_t3 INTEGER,
				f2_t4 INTEGER,
				f2_t5 INTEGER,
				f3_t1 INTEGER,
				f3_t2 INTEGER,
				f3_t3 INTEGER,
				f3_t4 INTEGER,
				f3_t5 INTEGER,
				f4_t1 INTEGER,
				f4_t2 INTEGER,
				f4_t3 INTEGER,
				f4_t4 INTEGER,
				f4_t5 INTEGER,
				Objective REAL
			);
		`)
		smt, err := db.Prepare("INSERT INTO Sweep VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);")
		fatalif(err)

		tx, err := db.Begin()
		stmt := tx.Stmt(smt)
		fatalif(err)

		for i, p := range buildSweep() {
			if i+1%100 == 0 {
				fatalif(tx.Commit())
				tx, err = db.Begin()
				stmt = tx.Stmt(smt)
				fatalif(err)
			}

			scen.InitParams(p)
			cycdb, simid, err := RunSim(scen)
			fatalif(err)
			val, err := CalcObjective(cycdb, simid, scen)
			fatalif(err)

			vals := []interface{}{}
			for _, v := range p {
				vals = append(vals, v)
			}
			vals = append(vals, val)

			_, err = stmt.Exec(vals...)
			fatalif(err)
		}
		fatalif(tx.Commit())
		return
	}

	if *metric != "" {
		// post process cyclus output db
		db, err := sql.Open("sqlite3", *metric)
		fatalif(err)
		defer db.Close()

		fatalif(post.Prepare(db))
		defer post.Finish(db)

		simids, err := post.GetSimIds(db)
		fatalif(err)

		ctx := post.NewContext(db, simids[0])
		if err := ctx.WalkAll(); err != nil {
			fmt.Println(err)
		}

		// calculate and write out objective val
		val, err := CalcObjective(*metric, simids[0], scen)
		fatalif(err)

		fmt.Println(val)
		return
	}

	// Parse dakota params into scenario object
	paramsFile := flag.Arg(0)
	err = ParseParams(scen, paramsFile)
	fatalif(err)

	cycout, simid, err := RunSim(scen)
	fatalif(err)

	// calculate and write out objective val
	resultFile := flag.Arg(1)
	val, err := CalcObjective(cycout, simid, scen)
	fatalif(err)

	err = ioutil.WriteFile(resultFile, []byte(fmt.Sprint(val)), 0755)
	fatalif(err)
}

func RunSim(scen *Scenario) (dbfile string, simid []byte, err error) {
	if scen.Handle == "" {
		scen.Handle = "none"
	}

	// generate cyclus input file and run cyclus
	ui := uuid.NewRandom()
	err = os.MkdirAll(tmpDir, 0755)
	if err != nil {
		return "", nil, err
	}

	cycin := filepath.Join(tmpDir, ui.String()+".cyclus.xml")
	cycout := filepath.Join(tmpDir, ui.String()+".sqlite")

	err = GenCyclusInfile(scen, cycin)
	if err != nil {
		return "", nil, err
	}

	cmd := exec.Command(scen.CyclusBin, cycin, "-o", cycout)
	cmd.Stderr = os.Stderr
	if err := cmd.Run(); err != nil {
		return "", nil, err
	}

	// post process cyclus output db
	db, err := sql.Open("sqlite3", cycout)
	if err != nil {
		return "", nil, err
	}
	defer db.Close()

	if err := post.Prepare(db); err != nil {
		return "", nil, err
	}
	defer post.Finish(db)

	simids, err := post.GetSimIds(db)
	if err != nil {
		return "", nil, err
	}

	ctx := post.NewContext(db, simids[0])
	if err := ctx.WalkAll(); err != nil {
		return "", nil, err
	}
	return cycout, simids[0], nil
}

func CalcObjective(dbfile string, simid []byte, scen *Scenario) (float64, error) {
	db, err := sql.Open("sqlite3", dbfile)
	if err != nil {
		return 0, err
	}

	// add up overnight and operating costs converted to PV(t=0)
	q1 := `
		SELECT tl.Time FROM TimeList AS tl
			INNER JOIN Agents As a ON a.EnterTime <= tl.Time AND (a.ExitTime >= tl.Time OR a.ExitTime IS NULL)
		WHERE
			a.SimId = tl.SimId AND a.SimId = ?
			AND a.Prototype = ?;
		`
	q2 := `SELECT EnterTime FROM Agents WHERE SimId = ? AND Prototype = ?`

	totcost := 0.0
	for _, fac := range scen.Facs {
		// calc total operating cost
		rows, err := db.Query(q1, simid, fac.Proto)
		if err != nil {
			return 0, err
		}
		for rows.Next() {
			var t int
			if err := rows.Scan(&t); err != nil {
				return 0, err
			}
			totcost += PV(fac.OpCost, t, scen.Discount)
		}
		if err := rows.Err(); err != nil {
			return 0, err
		}

		// calc overnight capital cost
		rows, err = db.Query(q2, simid, fac.Proto)
		if err != nil {
			return 0, err
		}
		for rows.Next() {
			var t int
			if err := rows.Scan(&t); err != nil {
				return 0, err
			}
			totcost += PV(fac.CapitalCost, t, scen.Discount)
		}
		if err := rows.Err(); err != nil {
			return 0, err
		}

		// add in waste penalty
		ags, err := query.AllAgents(db, simid, fac.Proto)
		if err != nil {
			return 0, err
		}

		// InvAt uses all agents if no ids are passed - so we need to skip from here
		if len(ags) == 0 {
			continue
		}

		ids := make([]int, len(ags))
		for i, a := range ags {
			ids[i] = a.Id
		}

		for t := 0; t < scen.SimDur; t++ {
			mat, err := query.InvAt(db, simid, t, ids...)
			if err != nil {
				return 0, err
			}
			for nuc, qty := range mat {
				nucstr := fmt.Sprint(nuc)
				totcost += PV(scen.NuclideCost[nucstr]*float64(qty)*(1-fac.WasteDiscount), t, scen.Discount)
			}
		}
	}

	// normalize to energy produced
	joules, err := query.EnergyProduced(db, simid, 0, scen.SimDur)
	if err != nil {
		return 0, err
	}
	mwh := joules / nuc.MWh
	mult := 1e6 // to get the objective around 0.1 same magnitude as constraint penalties
	return totcost / (mwh + 1e-30) * mult, nil
}

func PV(amt float64, nt int, rate float64) float64 {
	monrate := rate / 12
	return amt / math.Pow(1+monrate, float64(nt))
}

func ParseParams(scen *Scenario, fname string) error {
	data, err := ioutil.ReadFile(fname)
	if err != nil {
		return err
	}

	vals := []int{}
	h := ""
	s := string(data)
	lines := strings.Split(s, "\n")
	for i, l := range lines {
		fmt.Println(l)
		l = strings.TrimSpace(l)
		lines[i] = l
		fields := strings.Split(l, " ")
		for j, field := range fields {
			field = strings.TrimSpace(field)
			fields[j] = field
		}

		if len(fields) < 2 {
			continue
		}

		switch f := fields[1]; {
		case strings.HasPrefix(f, "b_f"):
			val, err := strconv.Atoi(fields[0])
			if err != nil {
				return err
			}
			vals = append(vals, val)
		case f == "eval_id":
			h = fields[0]
		}
	}
	scen.InitParams(vals)
	scen.Handle = h
	return nil
}

func GenCyclusInfile(scen *Scenario, fname string) error {
	tmpl := scen.CyclusTmpl
	t := template.Must(template.ParseFiles(tmpl))
	f, err := os.Create(fname)
	if err != nil {
		return err
	}
	defer f.Close()

	if err := t.Execute(f, scen); err != nil {
		return err
	}
	return nil
}

func fatalif(err error) {
	if err != nil {
		log.Fatal(err)
	}
}
