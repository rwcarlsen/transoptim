
Process
=========

Build the driver script::
    
    go build cycdriver.go

Enter custom parameters in prob-spec.json and then generate the dakota input::

    ./cycdriver -init

Run dakota::

    dakota -input [infile-name]

Where infile-name is the DakotaInfile parameter in the problem spec json file.

Dakota should output approximately (for the provided prob-spec.json)::

    blackbox evaluations                     : 37
    best feasible solution                   : ( 9 7 1 ) h=0 f=9.2307692308e-01
    <<<<< Function evaluation summary: 37 total (37 new, 0 duplicate)
    <<<<< Best parameters          =
                                         9 x0
                                         7 x1
                                         1 x2
    <<<<< Best objective function  =
                          9.2307692308e-01
    <<<<< Best data captured at function evaluation 29


    <<<<< Iterator mesh_adaptive_search completed.
    <<<<< Single Method Strategy completed.
    DAKOTA execution time in seconds:
      Total CPU        =   0.046693 [parent =   0.046666, child =    2.7e-05]
      Total wall clock =    32.4468

Note that all cyclus simulation iterations for an optimization run live in a
single output database safely.  However, multiple optimization runs in a single
database are NOT supported (yet).


Objective/Metric
==================

The cycdriver binary uses an objective function that approximates a global
capacity factor::

    M = [cumulative operating time of all agents] / [sim total transacted resource qty]

The goal is to minimize M.

Simulation structure
=====================

n source facilities are deployed at statically specified times defined in the
problem spec file. n initial deployment times are specified for sink
facilities. The optimizer tries to "discover" the best times to deploy n
sink facilities to minimize the objective. This problem structure was chosen
so that the global optimum solution (of sink facility build times) could
easily be analytically determined.  This allows correctness checking of the
dakota-cyclus optimization process.  The problem structure is such that the
optimum sink facility deployment schedule is identical to the statically
chosen source deployment schedule.

Provenance
===========

* DAKOTA Version: 5.4 released 11/15/2013. Subversion revision 2012 built Apr  1 2014 09:36:51.
* Cyclus Version: e8c090e39e0f14bd038e498147ec9e6c8fd3190f
* Cycamore Version: ab3b9c7cae2f1404a7cd1f1159286b2496590630
