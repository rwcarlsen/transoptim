package optim

import (
	"fmt"

	"github.com/gonum/matrix/mat64"
)

type Prototype string

type Depend struct {
	AtLeast   int
	AtLeastP  Prototype
	ForEvery  int
	ForEveryP Prototype
}

type Facility struct {
	Proto Prototype
	Cap   float64
	Life  int
}

type Scenario struct {
	SimDur int
	// BuildPeriod is the number of timesteps between timesteps in which
	// facilities are deployed
	BuildPeriod int
	Facs        []Facility
	Depends     []Depend
	Builds      []int
	MinPower    []float64
	MaxPower    []float64
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
