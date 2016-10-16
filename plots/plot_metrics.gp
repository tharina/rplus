set terminal pngcairo size 1024,768 enhanced font 'Verdana,14' linewidth 1.5
set output 'metrics.png'
#set terminal svg enhanced size 1024,768 fname 'Verdana' fsize 14 mouse jsdir '.' linewidth 1.5
#set output 'metrics.svg'

set pointsize 1.0
set grid xtics ytics
set offsets 0,0,graph 0.01,graph 0.01

set key inside top right
set xlabel 'number of points'
set ylabel 'running time per query and element (n) in nanoseconds'
set title 'uniform, running time'

set logscale x 2
set format x "2^{%L}"

plot  "data_metrics.txt" using 1:($2/$1/10000) title 'ds=Naive' with linespoints, \
	  "data_metrics.txt" using 1:($3/$1/10000) title 'ds=R+-Tree MinArea' with linespoints, \
      "data_metrics.txt" using 1:($4/$1/10000) title 'ds=R+-Tree MinCuts' with linespoints, \
