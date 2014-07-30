

binwidth=1
stats 'hist.dat' using 1 prefix "f"

bin(x,width)=width*floor(x/width)
set style fill solid border -1
set boxwidth 0.5*binwidth

show variables f

set ylabel 'Frequency'
set y2label 'Cumulative frequency (% total)'
set y2tics
set ytics nomirror
set xlabel 'Objective value (% over optimum)'

plot "hist.dat" using (bin(100*($1/f_min-1),binwidth)):(1.0) smooth frequency with boxes title "freq", \
     "" using (bin(100*($1/f_min-1),binwidth)):(100.0/f_records) axis x1y2 smooth cumulative lw 3 title "cumulative" with lines

pause -1

