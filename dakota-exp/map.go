package main

import (
	"database/sql"
	"flag"
	"fmt"
	"log"

	_ "github.com/mxk/go-sqlite/sqlite3"
)

type Point [14]int

// ReactorT1 int
// ReactorT2 int
// ReactorT3 int
// ReactorT4 int
// RepoT1 int
// RepoT2 int
// RepoT3 int
// RepoT4 int
// SepT2 int
// SepT3 int
// SepT4 int
// FuelT2 int
// FuelT3 int
// FuelT4 int

var points = make(map[Point]float64, 85000)

func main() {
	loadpoints()
	perms := Permute(3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3)
	for p, val := range points {
		fmt.Printf("checking f%v = %v\n", p, val)
		isoptim := true
		for _, perm := range perms {
			p2 := p
			for i, v := range perm {
				p2[i] += (v - 1)
			}
			if val2, ok := points[p2]; ok && val2 < val {
				isoptim = false
				break
			}
		}
		if isoptim {
			fmt.Println("    ** point is local optimum **")
		}
	}
}

func loadpoints() {
	flag.Parse()
	db, err := sql.Open("sqlite3", flag.Arg(0))
	if err != nil {
		log.Fatal(err)
	}
	defer db.Close()

	rows, err := db.Query("SELECT f1_t1,f1_t2,f1_t3,f1_t4,f2_t1,f2_t2,f2_t3,f2_t4,f3_t2,f3_t3,f3_t4,f4_t2,f4_t3,f4_t4,Objective FROM Sweep;")
	if err != nil {
		log.Fatal(err)
	}

	for rows.Next() {
		p := Point{}
		var val float64
		err := rows.Scan(
			&p[0],
			&p[1],
			&p[2],
			&p[3],
			&p[4],
			&p[5],
			&p[6],
			&p[7],
			&p[8],
			&p[9],
			&p[10],
			&p[11],
			&p[12],
			&p[13],
			&val,
		)
		if err != nil {
			log.Fatal(err)
		}
		points[p] = val
	}
	if err := rows.Err(); err != nil {
		log.Fatal(err)
	}

}

func Permute(dimensions ...int) [][]int {
	return permute(dimensions, []int{})
}

func permute(dimensions []int, prefix []int) [][]int {
	set := make([][]int, 0)

	if len(dimensions) == 1 {
		for i := 0; i < dimensions[0]; i++ {
			val := append(append([]int{}, prefix...), i)
			set = append(set, val)
		}
		return set
	}

	max := dimensions[0]
	for i := 0; i < max; i++ {
		newprefix := append(prefix, i)
		set = append(set, permute(dimensions[1:], newprefix)...)
	}
	return set
}
