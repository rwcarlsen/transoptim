package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io/ioutil"

	"github.com/gonum/blas/goblas"
	"github.com/gonum/matrix/mat64"
)

func init() {
	mat64.Register(goblas.Blas{})
}

// Facility represents a cyclus agent prototype that could be built by the
// optimizer.
type Facility struct {
	Proto string
	// Cap is the total Power output capacity of the facility.
	Cap float64
	// OpCost represents the per timstep operating cost for the facility
	OpCost float64
	// CapitalCost represents the overnight cost for building the facility
	CapitalCost float64
	// WasteCost represents a per timstep cost for the facility to store its
	// own waste.
	WasteCost float64
	// The lifetime of the facility (in timesteps). The lifetime must also
	// be specified manually (consistent with this value) in the prototype
	// definition in the cyclus input template file.
	Life int
}

// Alive returns whether or not a facility built at the specified time is
// still operating/active at t.
func (f *Facility) Alive(built, t int) bool {
	if built > t {
		return false
	}
	return built+f.Life >= t || f.Life <= 0
}

type Param struct {
	Time  int
	Proto string
	N     int
}

type Scenario struct {
	// SimDur is the simulation duration in timesteps (months)
	SimDur int
	// File is the name of the scenario file. This does not need to be filled
	// out by the user.
	File       string
	DakotaTmpl string
	CyclusTmpl string
	// CyclusBin is the full path to the cyclus binary
	CyclusBin string
	// BuildPeriod is the number of timesteps between timesteps in which
	// facilities are deployed
	BuildPeriod int
	// Discount represents the nominal annual discount rate (including
	// inflation) for the simulation.
	Discount float64
	Facs     []Facility
	// MinPower is a series of min deployed power capacity requirements that
	// must be maintained for each build period.
	MinPower []float64
	// MaxPower is a series of max deployed power capacity requirements that
	// must be maintained for each build period.
	MaxPower []float64
	// Params holds a set of potential build schedule values for the scenario.
	// This is for internal use and does not need to be specified by the user.
	Params []Param
	// Handle is used internally and does not need to be specified by the
	// user.
	Handle string
}

func (s *Scenario) Load(fname string) error {
	if s == nil {
		s = &Scenario{}
	}
	data, err := ioutil.ReadFile(fname)
	if err != nil {
		return err
	}
	if err := json.Unmarshal(data, s); err != nil {
		if serr, ok := err.(*json.SyntaxError); ok {
			line, col := findLine(data, serr.Offset)
			return fmt.Errorf("%s:%d:%d: %v", fname, line, col, err)
		}
		return err
	}

	s.File = fname
	s.Params = make([]Param, s.Nvars())
	return nil
}

func (s *Scenario) InitParams(vals []int) {
	s.Params = make([]Param, len(vals))
	for i, val := range vals {
		f := i / s.nPeriods()
		t := (i%s.nPeriods() + 1) * s.BuildPeriod
		s.Params[i].Time = t
		s.Params[i].Proto = s.Facs[f].Proto
		s.Params[i].N = val
	}
}

func (s *Scenario) VarNames() []string {
	nperiods := s.nPeriods()
	names := make([]string, s.Nvars())
	for f := range s.Facs {
		for n := 0; n < nperiods; n++ {
			i := f*nperiods + n
			names[i] = fmt.Sprintf("b_f%v_t%v", f, n)
		}
	}
	return names
}

func (s *Scenario) LowerBounds() *mat64.Dense {
	return mat64.NewDense(s.Nvars(), 1, nil)
}

func (s *Scenario) UpperBounds() *mat64.Dense {
	nperiods := s.nPeriods()
	up := mat64.NewDense(s.Nvars(), 1, nil)
	for f, fac := range s.Facs {
		for n := 0; n < nperiods; n++ {
			v := (s.MaxPower[n]/fac.Cap + 1)
			if fac.Cap != 0 {
				up.Set(f*nperiods+n, 0, v)
			} else {
				up.Set(f*nperiods+n, 0, 10)
			}
		}
	}
	return up
}

// SupportConstr builds and returns matrices representing linear inequality
// constraints with a parameter multiplier matrix A and upper and lower
// bounds. The constraint expresses that the total number of support
// facilities (i.e. not reactors) at every timestep must never be more
// than twice the number of deployed reactors.
func (s *Scenario) SupportConstr() (low, A, up *mat64.Dense) {
	nperiods := s.nPeriods()

	A = mat64.NewDense(nperiods, s.Nvars(), nil)
	low = mat64.NewDense(nperiods, 1, nil)
	tmp := make([]float64, len(s.MaxPower))
	copy(tmp, s.MaxPower)
	up = mat64.NewDense(nperiods, 1, tmp)
	up.Apply(func(r, c int, v float64) float64 { return 1e200 }, up)

	for t := s.BuildPeriod; t < s.SimDur; t += s.BuildPeriod {
		for f, fac := range s.Facs {
			for n := 0; n < nperiods; n++ {
				if !fac.Alive(n*s.BuildPeriod+1, t) {
					continue
				}

				i := f*nperiods + n
				if fac.Cap == 0 {
					A.Set(t/s.BuildPeriod-1, i, -1)
				} else {
					A.Set(t/s.BuildPeriod-1, i, 2)
				}
			}
		}
	}

	return low, A, up
}

// PowerConstr builds and returns matrices representing linear inequality
// constraints with a parameter multiplier matrix A and upper and lower
// bounds. The constraint expresses that the total power capacity deployed at
// every timestep must always be between the given MinPower and MaxPower
// scenario bounds.
func (s *Scenario) PowerConstr() (low, A, up *mat64.Dense) {
	nperiods := s.nPeriods()

	A = mat64.NewDense(nperiods, s.Nvars(), nil)
	tmp := make([]float64, len(s.MinPower))
	copy(tmp, s.MinPower)
	low = mat64.NewDense(nperiods, 1, tmp)
	copy(tmp, s.MaxPower)
	up = mat64.NewDense(nperiods, 1, tmp)

	for t := s.BuildPeriod; t < s.SimDur; t += s.BuildPeriod {
		for f, fac := range s.Facs {
			for n := 0; n < nperiods; n++ {
				if fac.Alive(n*s.BuildPeriod+1, t) {
					i := f*nperiods + n
					A.Set(t/s.BuildPeriod-1, i, fac.Cap)
				}
			}
		}
	}

	return low, A, up
}

func (s *Scenario) AllConstr() (low, A, up *mat64.Dense) {
	low, A, up = &mat64.Dense{}, &mat64.Dense{}, &mat64.Dense{}
	l1, a1, u1 := s.SupportConstr()
	l2, a2, u2 := s.PowerConstr()

	low.Stack(l1, l2)
	A.Stack(a1, a2)
	up.Stack(u1, u2)

	return low, A, up
}

func (s *Scenario) ConstrMat() (A *mat64.Dense) {
	_, A, _ = s.AllConstr()
	return A
}

func (s *Scenario) ConstrLow() (low *mat64.Dense) {
	low, _, _ = s.AllConstr()
	return low
}

func (s *Scenario) ConstrUp() (up *mat64.Dense) {
	_, _, up = s.AllConstr()
	return up
}

func (s *Scenario) Nvars() int {
	return s.nPeriods() * len(s.Facs)
}

func (s *Scenario) nPeriods() int {
	return (s.SimDur+1)/s.BuildPeriod - 1
}

func findLine(data []byte, pos int64) (line, col int) {
	line = 1
	buf := bytes.NewBuffer(data)
	for n := int64(0); n < pos; n++ {
		b, err := buf.ReadByte()
		if err != nil {
			panic(err) //I don't really see how this could happen
		}
		if b == '\n' {
			line++
			col = 1
		} else {
			col++
		}
	}
	return
}