package main

import (
	"database/sql"
	"encoding/json"
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"strconv"
	"strings"
	"text/template"

	_ "code.google.com/p/go-sqlite/go1/sqlite3"
)

var specfile = flag.String("spec", "prob-spec.json", "file containing problem specification")
var dakotaGen = flag.Bool("init", false, "true to generate the dakota input file")

type Spec map[string]interface{}

func (_ Spec) Decr(i int) int {
	return i - 1
}

func main() {
	flag.Parse()
	log.SetFlags(log.Lshortfile)

	paramsFile := flag.Arg(0)
	resultFile := flag.Arg(1)

	// load problem spec file
	spec := Spec{}
	spec["Spec"] = *specfile
	data, err := ioutil.ReadFile(*specfile)
	if err != nil {
		log.Fatal(err)
	}
	err = json.Unmarshal(data, &spec)
	if err != nil {
		log.Fatal(err)
	}

	if *dakotaGen {
		tfile := spec["DakotaTmpl"].(string)
		fname := spec["DakotaInfile"].(string)
		t := template.Must(template.ParseFiles(tfile))
		f, err := os.Create(fname)
		if err != nil {
			log.Fatal(err)
		}
		defer f.Close()

		if err := t.Execute(f, spec); err != nil {
			log.Fatal(err)
		}
		return
	}

	// build cyclus input file from dakota params and problem spec
	err = ParseParams(spec, paramsFile)
	if err != nil {
		log.Fatal(err)
	}

	err = GenCyclusInfile(spec)
	if err != nil {
		log.Fatal(err)
	}

	// run cyclus
	fname := spec["CyclusInfile"].(string)
	bin := spec["CyclusBin"].(string)
	cmd := exec.Command(bin, "--flat-schema", fname)

	if err := cmd.Run(); err != nil {
		log.Fatal(err)
	}

	// calculate objective and write to results file
	h := spec["Handle"].(string)
	db, err := sql.Open("sqlite3", "cyclus.sqlite")
	if err != nil {
		log.Fatal(err)
	}

	q := `
		SELECT SUM(res.Quantity) FROM
			Transactions AS tr
			INNER JOIN Resources AS res
			INNER JOIN Info
		WHERE
			res.SimId = tr.SimId AND Info.SimId = tr.SimId
			AND tr.ResourceId = res.ResourceId
			AND Info.Handle = ?
		`
	row := db.QueryRow(q, h)
	transQty := sql.NullInt64{}
	if err := row.Scan(&transQty); err != nil {
		log.Fatal(err)
	}

	q = `
		SELECT SUM(Info.Duration-ae.EnterTime) FROM
			AgentEntry AS ae
			INNER JOIN Info
		WHERE
			ae.SimId = Info.SimId
			AND Info.Handle = ?
		`
	row = db.QueryRow(q, h)
	runTime := sql.NullInt64{}
	if err := row.Scan(&runTime); err != nil {
		log.Fatal(err)
	}

	objective := float64(runTime.Int64) / float64(transQty.Int64)

	err = ioutil.WriteFile(resultFile, []byte(fmt.Sprint(objective)), 0755)
	if err != nil {
		log.Fatal(err)
	}
}

func ParseParams(spec Spec, fname string) error {
	data, err := ioutil.ReadFile(fname)
	if err != nil {
		return err
	}

	vals := []int{}
	h := ""
	s := string(data)
	lines := strings.Split(s, "\n")
	for i, l := range lines {
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
		case strings.HasPrefix(f, "x"):
			val, err := strconv.ParseFloat(fields[0], 64)
			if err != nil {
				return err
			}
			vals = append(vals, int(val))
			fmt.Printf("%v: %v\n", fields[1], val)
		case f == "eval_id":
			h = fields[0]
		}
	}
	spec["Params"] = vals
	spec["Handle"] = h
	return nil
}

type Filler struct {
	Handle     string
	SourceFacs []int
	SinkFacs   []int
}

func GenCyclusInfile(spec Spec) error {
	cyclusTmpl := spec["CyclusTmpl"].(string)
	cyclusInfile := spec["CyclusInfile"].(string)
	t := template.Must(template.ParseFiles(cyclusTmpl))
	f, err := os.Create(cyclusInfile)
	if err != nil {
		return err
	}
	defer f.Close()

	if err := t.Execute(f, spec); err != nil {
		return err
	}
	return nil
}
