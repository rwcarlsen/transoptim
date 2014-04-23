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
)

var scenfile = flag.String("scen", "scenario.json", "file containing problem scenification")
var dakotaGen = flag.Bool("init", false, "true to generate the dakota input file")
var dakotaif = flag.String("o", "", "name of generated dakota input file")

const tmpDir = "cyctmp"

func main() {
	flag.Parse()
	log.SetFlags(log.Lshortfile)

	// load problem scen file
	scen := &Scenario{}
	err := scen.Load(*scenfile)
	if err != nil {
		log.Fatal(err)
	}

	// build dakota input file from template and return
	if *dakotaGen {
		if *dakotaif == "" {
			*dakotaif = filepath.Base(scen.DakotaTmpl) + ".gen"
		}

		t := template.Must(template.ParseFiles(scen.DakotaTmpl))
		f, err := os.Create(*dakotaif)
		if err != nil {
			log.Fatal(err)
		}
		defer f.Close()

		if err := t.Execute(f, scen); err != nil {
			log.Fatal(err)
		}
		return
	}

	// Parse dakota params into scenario object
	paramsFile := flag.Arg(0)
	err = ParseParams(scen, paramsFile)
	if err != nil {
		log.Fatal(err)
	}

	// generate cyclus input file and run cyclus and post process db
	ui := uuid.NewRandom()
	if err := os.MkdirAll(tmpDir, 0755); err != nil {
		log.Fatal(err)
	}
	cycin := filepath.Join(tmpDir, ui.String()+".cyclus.xml")
	cycout := filepath.Join(tmpDir, ui.String()+".sqlite")

	err = GenCyclusInfile(scen, cycin)
	if err != nil {
		log.Fatal(err)
	}

	cmd := exec.Command(scen.CyclusBin, "--flat-schema", cycin, "-o", cycout)
	cmd.Stderr = os.Stderr
	if err := cmd.Run(); err != nil {
		log.Fatal(err)
	}

	cmd = exec.Command("inventory", cycout)
	cmd.Stderr = os.Stderr
	if err := cmd.Run(); err != nil {
		log.Fatal(err)
	}

	// calculate and write out objective val
	resultFile := flag.Arg(1)
	val, err := CalcObjective(cycout, scen.Handle, scen)
	if err != nil {
		log.Fatal(err)
	}

	err = ioutil.WriteFile(resultFile, []byte(fmt.Sprint(val)), 0755)
	if err != nil {
		log.Fatal(err)
	}
}

func CalcObjective(dbfile, handle string, scen *Scenario) (float64, error) {
	db, err := sql.Open("sqlite3", dbfile)
	if err != nil {
		return 0, err
	}

	q1 := `
		SELECT tl.Time FROM TimeList AS tl
			INNER JOIN Agents As a ON a.EnterTime <= tl.Time AND (a.ExitTime >= tl.Time OR a.ExitTime IS NULL)
			INNER JOIN Info
		WHERE
			a.SimId = tl.SimId AND a.SimId = Info.SimId AND Info.Handle = ?
			AND a.Prototype = ?;
		`
	q2 := `
		SELECT a.EnterTime FROM Agents AS a
			INNER JOIN Info ON Info.SimId = a.SimId
		WHERE
			Info.Handle = ?
			AND a.Prototype = ?
		`

	totcost := 0.0
	for _, fac := range scen.Facs {
		rows, err := db.Query(q1, handle, fac.Proto)
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

		rows, err = db.Query(q2, handle, fac.Proto)
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
	}

	return totcost, nil
}

func PV(amt float64, nt int, rate float64) float64 {
	return amt / math.Pow(1+rate, float64(nt))
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
