package optim

import "testing"

func TestVarNames(t *testing.T) {
	facs := []Facility{
		Facility{"Proto1", 10, 4},
		Facility{"Proto2", 5, 6},
		Facility{"Proto3", 5, 2},
	}
	s := &Scenario{
		SimDur:      10,
		BuildPeriod: 2,
		Facs:        facs,
		MaxPower:    []float64{50, 50, 50, 50, 50, 50},
	}

	t.Logf("Scenario: %+v", s)
	t.Logf("nVars: %+v", s.nVars())
	t.Logf("nPeriods: %+v", s.nPeriods())
	t.Logf("VarNames: %+v", s.VarNames())
	t.Logf("LowerBounds: %+v", s.LowerBounds())
	t.Logf("UpperBounds: %+v", s.UpperBounds())
	t.Logf("UpperBounds: %+v", s.PowerConstr())
}
