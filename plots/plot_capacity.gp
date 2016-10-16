set terminal pngcairo size 1024,768 enhanced font 'Verdana,14' linewidth 1.5
set output 'capacity.png'
#set terminal svg enhanced size 1024,768 fname 'Verdana' fsize 14 mouse jsdir '.' linewidth 1.5
#set output 'capacity.svg'

set pointsize 1.0
set grid xtics ytics
set offsets 0,0,graph 0.01,graph 0.01

set key inside top right
set xlabel 'Node Capacity'
set ylabel 'running time per query and element (n) in nanoseconds'
set title 'uniform, running time'

set logscale x 2
plot  "data_capacity.txt" using 1:($2/(2**15)/10000) title 'R+-Tree with 2^{15} points' with linespoints, \
	  "data_capacity.txt" using 1:($3/(2**17)/10000) title 'R+-Tree with 2^{17} points' with linespoints, \
      "data_capacity.txt" using 1:($4/(2**19)/10000) title 'R+-Tree with 2^{19} points' with linespoints, \
