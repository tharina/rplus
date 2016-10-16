set terminal pngcairo size 1024,768 enhanced font 'Verdana,14' linewidth 1.5
set output 'mem.png'
#set terminal svg enhanced size 1024,768 fname 'Verdana' fsize 14 mouse jsdir '.' linewidth 1.5
#set output 'mem.svg'

set pointsize 1.0
set grid xtics ytics
set offsets 0,0,graph 0.01,graph 0.01

set key inside bottom right
set xlabel 'Number of points'
set ylabel 'memory allocation per element (n) in bytes'
set title 'uniform, peak memory usage'


set logscale x 2
set format x "2^{%L}"
#set logscale y

plot  "data_mem.txt" using 1:($2/$1) title 'ds=R+-Tree' with linespoints, \
