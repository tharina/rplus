set terminal svg enhanced size 1024,768 fname 'Verdana' fsize 10 mouse jsdir '.' linewidth 1.5
set output 'out.svg'

set pointsize 1.0
set grid xtics ytics
set offsets 0,0,graph 0.01,graph 0.01

set key inside top right
set xlabel 'Node Capacity'
set ylabel 'time per element (n) in nanoseconds'
set title 'uniform, running time'

set logscale x 2
plot  "data.txt" using 1:($2/131072/10000) title 'R+-Tree with 2^{17} points' with linespoints
