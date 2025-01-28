
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
set size square
file_exists(file) = system("[ -f '".file."' ] && echo '1' || echo '0'") + 0

if (file_exists("NEAREST.STAGNATION.out")){
   stagnationNearest=`cat NEAREST.STAGNATION.out 2>/dev/null`
   set arrow from stagnationNearest, graph 0 to stagnationNearest, graph 1 nohead  linecolor "dark-green" dashtype 4 lw 2
}

set output "stagnationLog-0.pdf"
plot 100000+ 0.1*x title 'reference' lc black,\
     "NEAREST.out"     using 1:2 notitle lc "dark-green" pt 2


set output "stagnationLog-1.pdf"
plot  100000+ 0.1*x title 'reference' lc black,\
      "NEAREST.out"      using 1:2 notitle lc "dark-green" pt 2 ps 0.7,\
      "RANDOM.0.out"     using 1:2 notitle lc "dark-red"   pt 2 ps 0.7,\
      "AVERAGE.0.out"    using 1:2 notitle lc "blue"       pt 2 ps 0.7


if (file_exists("RANDOM_DET.STAGNATION.out")) {
   stagnationRandomDet=`cat RANDOM_DET.STAGNATION.out |head -n 1 2>/dev/null`
   set arrow from stagnationRandomDet, graph 0 to stagnationRandomDet, graph 1 nohead  linecolor "dark-violet" dashtype 2 lw 2
}
if (file_exists("AVERAGE_DET.STAGNATION.out")) {
   stagnationAverageDet=`cat AVERAGE_DET.STAGNATION.out |head -n 1 2>/dev/null`
   set arrow from stagnationAverageDet, graph 0 to stagnationAverageDet, graph 1 nohead  linecolor 0x008B8B dashtype 5 lw 2
}


set output "stagnationLog-2.pdf"
plot 100000+ 0.1*x title 'reference' lc black,\
     "NEAREST.out"       using 1:2 notitle lc "dark-green"  pt 2 ps 0.7,\
     "RANDOM.0.out"      using 1:2 notitle lc "dark-red"    pt 2 ps 0.7,\
     "AVERAGE.0.out"     using 1:2 notitle lc "blue"        pt 2 ps 0.7,\
     "RANDOM_DET.0.out"  using 1:2 notitle lc "dark-violet" pt 2 ps 0.7 ,\
     "AVERAGE_DET.0.out" using 1:2 notitle lc 0x008B8B      pt 2 ps 0.7

if (file_exists("SR_MONOTONIC.STAGNATION.out")){
   stagnationSRMono=`cat SR_MONOTONIC.STAGNATION.out | head -n 1 2>/dev/null`
   set arrow from stagnationSRMono, graph 0 to stagnationSRMono, graph 1 nohead  linecolor "red" dashtype 7 lw 2
}

set output "stagnationLog.pdf"
plot      100000+ 0.1*x title 'reference' lc black,\
     "NEAREST.out"        using 1:2 notitle lc "dark-green"  pt 2 ps 0.7,\
     "RANDOM.0.out"       using 1:2 notitle lc "dark-red"    pt 2 ps 0.7,\
     "AVERAGE.0.out"      using 1:2 notitle lc "blue"        pt 2 ps 0.7,\
     "RANDOM_DET.0.out"   using 1:2 notitle lc "dark-violet" pt 2 ps 0.7,\
     "AVERAGE_DET.0.out"  using 1:2 notitle lc 0x008B8B      pt 2 ps 0.7,\
     "SR_MONOTONIC.0.out" using 1:2 notitle lc "red"         pt 2 ps 0.7

