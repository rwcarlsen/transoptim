package optim

import "fmt"

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

func (s *Scenario) LowerBounds() []float64 {
	return make([]float64, s.nVars())
}

func (s *Scenario) UpperBounds() []float64 {
	nperiods := s.nPeriods()
	up := make([]float64, s.nVars())
	for f, fac := range s.Facs {
		for n := 0; n < nperiods; n++ {
			i := f*nperiods + n
			up[i] = (s.MaxPower[n]/fac.Cap + 1) * 2
		}
	}
	return up
}

func (s *Scenario) PowerConstr() (A [][]float64, b []float64) {
	nperiods := s.nPeriods()
	cs := make([][]float64, 0, nperiods)
	for t := 0; t < s.SimDur; t += s.BuildPeriod {
		c := make([]float64, s.nVars())
		for f, fac := range s.Facs {
			for n := 0; n < nperiods; n++ {
				i := f*nperiods + n
				if n*s.BuildPeriod+fac.Life < t {
					c[i] = 0
				} else {
					c[i] = fac.Cap
				}
			}
		}
		cs = append(cs, c)
	}
	return cs
}

func (s *Scenario) nVars() int {
	return s.nPeriods() * len(s.Facs)
}

func (s *Scenario) nPeriods() int {
	return s.SimDur/s.BuildPeriod + 1
}
