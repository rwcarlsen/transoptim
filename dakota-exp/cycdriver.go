package main

import (
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"path/filepath"
	"strconv"
	"strings"
	"text/template"

	_ "code.google.com/p/go-sqlite/go1/sqlite3"
)

var scenfile = flag.String("scen", "scenario.json", "file containing problem scenification")
var dakotaGen = flag.Bool("init", false, "true to generate the dakota input file")
var dakotaif = flag.String("o", "", "name of generated dakota input file")

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
	//resultFile := flag.Arg(1)

	err = ParseParams(scen, paramsFile)
	if err != nil {
		log.Fatal(err)
	}

	// generate cyclus input file
	cycfile, err := GenCyclusInfile(scen)
	if err != nil {
		log.Fatal(err)
	}

	// run cyclus
	bin := scen.CyclusBin
	cmd := exec.Command(bin, "--flat-schema", cycfile)

	if err := cmd.Run(); err != nil {
		log.Fatal(err)
	}

	// calculate objective and write to results file
	//h := scen["Handle"].(string)
	//db, err := sql.Open("sqlite3", "cyclus.sqlite")
	//if err != nil {
	//	log.Fatal(err)
	//}

	//q := `
	//	SELECT SUM(res.Quantity) FROM
	//		Transactions AS tr
	//		INNER JOIN Resources AS res
	//		INNER JOIN Info
	//	WHERE
	//		res.SimId = tr.SimId AND Info.SimId = tr.SimId
	//		AND tr.ResourceId = res.ResourceId
	//		AND Info.Handle = ?
	//	`
	//row := db.QueryRow(q, h)
	//transQty := sql.NullInt64{}
	//if err := row.Scan(&transQty); err != nil {
	//	log.Fatal(err)
	//}

	//q = `
	//	SELECT SUM(Info.Duration-ae.EnterTime) FROM
	//		AgentEntry AS ae
	//		INNER JOIN Info
	//	WHERE
	//		ae.SimId = Info.SimId
	//		AND Info.Handle = ?
	//	`
	//row = db.QueryRow(q, h)
	//runTime := sql.NullInt64{}
	//if err := row.Scan(&runTime); err != nil {
	//	log.Fatal(err)
	//}

	//objective := float64(runTime.Int64) / float64(transQty.Int64)

	//err = ioutil.WriteFile(resultFile, []byte(fmt.Sprint(objective)), 0755)
	//if err != nil {
	//	log.Fatal(err)
	//}
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

func GenCyclusInfile(scen *Scenario) (cycfile string, err error) {
	tmpl := scen.CyclusTmpl
	fname := tmpl + ".gen"
	t := template.Must(template.ParseFiles(tmpl))
	f, err := os.Create(fname)
	if err != nil {
		return "", err
	}
	defer f.Close()

	if err := t.Execute(f, scen); err != nil {
		return "", err
	}
	return fname, nil
}
