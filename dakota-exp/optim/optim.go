package optim

import (
	"encoding/json"
	"fmt"
	"io/ioutil"

	"github.com/gonum/matrix/mat64"
)

type Prototype string

type Facility struct {
	Proto Prototype
	Cap   float64
	Life  int
}

type Param struct {
	Time  int
	Proto string
	N     int
}

type Scenario struct {
	SimDur int
	// BuildPeriod is the number of timesteps between timesteps in which
	// facilities are deployed
	BuildPeriod int
	Facs        []Facility
	Builds      []int
	MinPower    []float64
	MaxPower    []float64
	// Params holds a set of potential build schedule values for the scenario.
	Params []Param
}

func (s *Scenario) Load(fname string) error {
	data, err := ioutil.ReadFile(fname)
	if err != nil {
		return err
	}
	return json.Unmarshal(data, s)
}

func (s *Scenario) VarNames() []string {
	nperiods := s.nPeriods()
	names := make([]string, s.nVars())
	for f := range s.Facs {
		for n := 0; n < nperiods; n++ {
			i := f*nperiods + n
			names[i] = fmt.Sprintf("b_f%v_t%v", f, n)
		}
	}
	return names
}

func (s *Scenario) LowerBounds() *mat64.Dense {
	return mat64.NewDense(s.nVars(), 1, nil)
}

func (s *Scenario) UpperBounds() *mat64.Dense {
	nperiods := s.nPeriods()
	up := mat64.NewDense(s.nVars(), 1, nil)
	for f, fac := range s.Facs {
		for n := 0; n < nperiods; n++ {
			if fac.Cap != 0 {
				up.Set(f*nperiods+n, 0, (s.MaxPower[n]/fac.Cap+1)*2)
			} else {
				up.Set(f*nperiods+n, 0, 1000)
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

	A = mat64.NewDense(nperiods, s.nVars(), nil)
	low = mat64.NewDense(nperiods, 1, nil)
	up = mat64.NewDense(nperiods, 1, s.MaxPower)
	up.Apply(func(r, c int, v float64) float64 { return 1e200 }, up)

	for t := 0; t < s.SimDur; t += s.BuildPeriod {
		for f, fac := range s.Facs {
			for n := 0; n < nperiods; n++ {
				alive := n*s.BuildPeriod+fac.Life >= t && n*s.BuildPeriod <= t
				if !alive {
					continue
				}

				i := f*nperiods + n
				if fac.Cap == 0 {
					A.Set(t/s.BuildPeriod, i, -1)
				} else {
					A.Set(t/s.BuildPeriod, i, 2)
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

	A = mat64.NewDense(nperiods, s.nVars(), nil)
	low = mat64.NewDense(nperiods, 1, s.MinPower)
	up = mat64.NewDense(nperiods, 1, s.MaxPower)

	for t := 0; t < s.SimDur; t += s.BuildPeriod {
		for f, fac := range s.Facs {
			for n := 0; n < nperiods; n++ {
				if n*s.BuildPeriod+fac.Life >= t && n*s.BuildPeriod <= t {
					i := f*nperiods + n
					A.Set(t/s.BuildPeriod, i, fac.Cap)
				}
			}
		}
	}

	return low, A, up
}

func (s *Scenario) nVars() int {
	return s.nPeriods() * len(s.Facs)
}

func (s *Scenario) nPeriods() int {
	return (s.SimDur + 1) / s.BuildPeriod
}
