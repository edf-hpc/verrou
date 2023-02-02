
set terminal pdf


set logscale x
set logscale y
set yrange [100000:1.e8]
set xrange [1000000:1.e8]
set ylabel "accumulator value"
set xlabel "increment number"

set format y "10^{%L}"
set key top left
#set key at 14000000, 0.0000004
set grid

stagnationNearest=`cat NEAREST.STAGNATION.out`
#stagnationRandomDet=`cat RANDOM_DET.STAGNATION.out`
stagnationAverageDet=`cat AVERAGE_DET.STAGNATION.out`
stagnationSRMono=`cat SR_MONOTONIC.STAGNATION.out`


set size square

set arrow from stagnationNearest, graph 0 to stagnationNearest, graph 1 nohead  linecolor "dark-green" dashtype 4 lw 2

set output "stagnationLog-0.pdf"
plot 100000+ 0.1*x title 'reference' lc black,\
     "NEAREST.out"     using 1:2 title 'nearest' lc "dark-green" pt 3


set output "stagnationLog-1.pdf"
plot  100000+ 0.1*x title 'reference' lc black,\
      "NEAREST.out"     using 1:2 title 'nearest' lc "dark-green" pt 3	,\
     "RANDOM.out"     using 1:2 title 'random' lc "dark-red" pt 1 ,\
     "AVERAGE.out"     using 1:2 title 'average' lc "blue" pt 1



#set arrow from stagnationRandomDet, graph 0 to stagnationRandomDet, graph 1 nohead  linecolor "orange-red" dashtype 2 lw 2

set arrow from stagnationAverageDet, graph 0 to stagnationAverageDet, graph 1 nohead  linecolor 0x008B8B dashtype 5 lw 2

set output "stagnationLog-2.pdf"
plot 100000+ 0.1*x title 'reference' lc black,\
     "NEAREST.out"     using 1:2 title 'nearest' lc "dark-green" pt 3	,\
     "RANDOM.out"     using 1:2 title 'random' lc "dark-red" pt 1 ,\
     "AVERAGE.out"     using 1:2 title 'average' lc "blue" pt 1,\
     "RANDOM_DET.out" using 1:2 title 'random\_det' lc "orange-red" pt 12,\
     "AVERAGE_DET.out" using 1:2 title 'average\_det' lc 0x008B8B pt 4

set arrow from stagnationSRMono, graph 0 to stagnationSRMono, graph 1 nohead  linecolor "red" dashtype 7 lw 2

set output "stagnationLog.pdf"
plot      100000+ 0.1*x title 'reference' lc black,\
     "NEAREST.out"     using 1:2 title 'nearest' lc "dark-green" pt 3	,\
     "RANDOM.out"     using 1:2 title 'random' lc "dark-red" pt 1 ,\
     "AVERAGE.out"     using 1:2 title 'average' lc "blue" pt 1,\
     "RANDOM_DET.out" using 1:2 title 'random\_det' lc "orange-red" pt 12,\
     "AVERAGE_DET.out" using 1:2 title 'average\_det' lc 0x008B8B pt 4 ,\
     "SR_MONOTONIC.out" using 1:2 title 'random\_monotonic' lc "red" pt 6
