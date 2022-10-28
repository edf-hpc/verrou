
set terminal pdf


set logscale x
set logscale y
set yrange [1e-7:1]
set xrange [1000:1.e8]
set ylabel "relative error"
set xlabel "increment number"

set format y "10^{%L}"
set key top left
#set key at 14000000, 0.0000004
set grid

stagnationAverageDet=`cat AVERAGE_DET.STAGNATION.out`
stagnationRandomDet=`cat RANDOM_DET.STAGNATION.out`
stagnationNearest=`cat NEAREST.STAGNATION.out`




set arrow from stagnationNearest, graph 0 to stagnationNearest, graph 1 nohead  linecolor "dark-green" dashtype 4 lw 2


set output "stagnationErrorLog-0.pdf"
plot "NEAREST.out"     using 1:4 title 'nearest' lc "dark-green" pt 3

set output "stagnationErrorLog-1.pdf"
plot "NEAREST.out"     using 1:4 title 'nearest' lc "dark-green" pt 3	,\
     "RANDOM.out"     using 1:4 title 'random' lc "dark-red" pt 1 ,\
     "AVERAGE.out"     using 1:4 title 'average' lc "blue" pt 1


set arrow from stagnationAverageDet, graph 0 to stagnationAverageDet, graph 1 nohead  linecolor 0x008B8B dashtype 5 lw 2
set arrow from stagnationRandomDet, graph 0 to stagnationRandomDet, graph 1 nohead  linecolor "orange-red" dashtype 2 lw 2

set output "stagnationErrorLog.pdf"
plot "NEAREST.out"     using 1:4 title 'nearest' lc "dark-green" pt 3	,\
     "RANDOM.out"     using 1:4 title 'random' lc "dark-red" pt 1 ,\
     "AVERAGE.out"     using 1:4 title 'average' lc "blue" pt 1,\
     "RANDOM_DET.out" using 1:4 title 'random\_det' lc "orange-red" pt 12,\
     "AVERAGE_DET.out" using 1:4 title 'average\_det' lc 0x008B8B pt 4
