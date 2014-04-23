package main

import (
	"fmt"
	"testing"

	"github.com/gonum/matrix/mat64"
)

func TestVarNames(t *testing.T) {
	facs := []Facility{
		Facility{"Proto1", 10, 3},
		Facility{"Proto2", 5, 3},
		Facility{"Proto3", 5, 5},
		Facility{"Proto4", 0, 3},
	}
	s := &Scenario{
		SimDur:      10,
		BuildPeriod: 2,
		Facs:        facs,
		MinPower:    []float64{10, 20, 30, 50},
		MaxPower:    []float64{150, 150, 150, 150},
	}

	t.Logf("Scenario: %+v", s)
	for _, fac := range s.Facs {
		t.Logf("   %+v", fac)
	}
	t.Logf("Nvars: %+v", s.Nvars())
	t.Logf("nPeriods: %+v", s.nPeriods())

	t.Log("VarNames:")
	for i, name := range s.VarNames() {
		t.Logf("   %v| %v", i, name)
	}
	t.Logf("LowerBounds:\n%v", Mat{s.LowerBounds()})
	t.Logf("UpperBounds:\n%v", Mat{s.UpperBounds()})

	low, Ap, up := s.PowerConstr()
	t.Log("Power Constraints:")
	t.Logf("    LowerBounds:\n%v", Mat{low})
	t.Logf("    UpperBounds:\n%v", Mat{up})
	t.Logf("    A:\n%v", Mat{Ap})

	low, As, up := s.SupportConstr()
	t.Log("Support Constraints:")
	t.Logf("    LowerBounds:\n%v", Mat{low})
	t.Logf("    UpperBounds:\n%v", Mat{up})
	t.Logf("    A:\n%v", Mat{As})

	l, A, u := s.AllConstr()
	t.Log("All Constraints:")
	t.Logf("    LowerBounds:\n%v", Mat{l})
	t.Logf("    UpperBounds:\n%v", Mat{u})
	t.Logf("    A:\n%v", Mat{A})
}

type Mat struct {
	*mat64.Dense
}

func (m Mat) Format(f fmt.State, c rune) {
	mat64.Format(m, 0, 0, f, c)
}
