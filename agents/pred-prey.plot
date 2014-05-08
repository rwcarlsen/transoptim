
set title "Predator Prey Dynamics in Cyclus"
set xlabel "Time"
set ylabel "Population"
plot "rabbit.dat" title "rabbit" with lp, \
     "wolf.dat" title "wolf" with lp
pause -1
set term svg size 1200,700
set output "pred-prey.svg"
plot "rabbit.dat" title "rabbit" with lp, \
     "wolf.dat" title "wolf" with lp
