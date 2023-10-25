
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

file_exists(file) = system("[ -f '".file."' ] && echo '1' || echo '0'") + 0


set pointsize 0.75
set label "nearest" at      1400,0.40


if (file_exists("NEAREST.STAGNATION.out")){
   stagnationNearest=`cat NEAREST.STAGNATION.out 2>/dev/null`
   set arrow from stagnationNearest, graph 0 to stagnationNearest, graph 1 nohead  linecolor "dark-green" dashtype 4 lw 2
}

set output "stagnationErrorLog-0.pdf"

plot "NEAREST.out"       using 1:4 notitle  lc "dark-green" pt 2 ,\
      "<echo '15000 0.40'" notitle lc "dark-green" pt 2 ,\


set label "random" at	    1400,0.2
set label "average" at      1400,0.1
set label "first" at      12000,0.65 font "sans,10"
set label "mean" at       22000,0.65 font "sans,10"
set label "max" at        50000,0.65 font "sans,10"


set output "stagnationErrorLog-1.pdf"
plot "NEAREST.out"       using 1:4 notitle  lc "dark-green" pt 2 ps 0.8,\
     "RANDOM.0.out"      using 1:4 notitle  lc "dark-red" pt 2 ps 0.8,\
     "RANDOM.post.out"   using 1:3 notitle  lc "dark-red" pt 6 ps 0.5 ,\
     "RANDOM.post.out"   using 1:7 notitle  with line lc "dark-red" lt 3,\
     "AVERAGE.0.out"      using 1:4 notitle  lc "blue" pt 2 ps 0.8,\
     "AVERAGE.post.out"   using 1:3 notitle  lc "blue" pt 6 ps 0.5 ,\
     "AVERAGE.post.out"   using 1:7 notitle  with line lc "blue" lt 3,\
      "<echo '15000 0.40'" notitle lc "dark-green" pt 2 ,\
      "<echo '15000 0.2'" notitle lc "dark-red" pt 2 ,\
      "<echo '30000 0.2'" notitle lc "dark-red" pt 6 ps 0.5 ,\
      "<echo '50000 0.2\n90000 0.2'" notitle  with line lc "dark-red"  lt 3 ,\
      "<echo '15000 0.1'" notitle lc "blue" pt 2 ,\
      "<echo '30000 0.1'" notitle lc "blue" pt 6 ps 0.5 ,\
      "<echo '50000 0.1\n90000 0.1'" notitle  with line lc "blue"  lt 3 ,\



if (file_exists("RANDOM_DET.STAGNATION.out")) {
   stagnationRandomDet=`cat RANDOM_DET.STAGNATION.out |head -n 1 2>/dev/null`
   set arrow from stagnationRandomDet, graph 0 to stagnationRandomDet, graph 1 nohead  linecolor "dark-violet" dashtype 2 lw 2
}
if (file_exists("AVERAGE_DET.STAGNATION.out")) {
   stagnationAverageDet=`cat AVERAGE_DET.STAGNATION.out |head -n 1 2>/dev/null`
   set arrow from stagnationAverageDet, graph 0 to stagnationAverageDet, graph 1 nohead  linecolor 0x008B8B dashtype 5 lw 2
}

set label "random\_det" noenhanced at	    1400,0.05
set label "average\_det"  noenhanced  at 1400,0.025

set output "stagnationErrorLog-2.pdf"
plot "NEAREST.out"       using 1:4 notitle  lc "dark-green" pt 2 ps 0.8,\
     "RANDOM.0.out"      using 1:4 notitle  lc "dark-red" pt 2 ps 0.8,\
     "RANDOM.post.out"   using 1:3 notitle  lc "dark-red" pt 6 ps 0.5 ,\
     "RANDOM.post.out"   using 1:7 notitle  with line lc "dark-red" lt 3,\
     "AVERAGE.0.out"     using 1:4 notitle  lc "blue" pt 2 ps 0.8,\
     "AVERAGE.post.out"  using 1:3 notitle lc "blue" pt 6  ps 0.5 ,\
     "AVERAGE.post.out"  using 1:7 notitle with line lc "blue" lt 3,\
     "RANDOM_DET.0.out"      using 1:4 notitle  lc "dark-violet" pt 2 ps 0.8 ,\
     "RANDOM_DET.post.out"   using 1:3 notitle  lc "dark-violet" pt 6 ps 0.5 ,\
     "RANDOM_DET.post.out"   using 1:7 notitle  with line lc "dark-violet" lt 3,\
     "AVERAGE_DET.0.out"     using 1:4 notitle lc 0x008B8B pt 2 ps 0.8,\
     "AVERAGE_DET.post.out"  using 1:3 notitle lc 0x008B8B pt 6  ps 0.5,\
     "AVERAGE_DET.post.out"  using 1:7 notitle  with line lc 0x008B8B  lt 3 ,\
      "<echo '15000 0.40'" notitle lc "dark-green" pt 2 ,\
      "<echo '15000 0.2'" notitle lc "dark-red" pt 2 ,\
      "<echo '30000 0.2'" notitle lc "dark-red" pt 6 ps 0.5 ,\
      "<echo '50000 0.2\n90000 0.2'" notitle  with line lc "dark-red"  lt 3 ,\
      "<echo '15000 0.1'" notitle lc "blue" pt 2 ,\
      "<echo '30000 0.1'" notitle lc "blue" pt 6 ps 0.5 ,\
      "<echo '50000 0.1\n90000 0.1'" notitle  with line lc "blue"  lt 3 ,\
      "<echo '15000 0.05'" notitle lc "dark-violet" pt 2 ,\
      "<echo '30000 0.05'" notitle lc "dark-violet" pt 6 ps 0.5 ,\
      "<echo '50000 0.05\n90000 0.05'" notitle  with line lc "dark-violet"  lt 3 ,\
      "<echo '15000 0.025'" notitle lc 0x008B8B pt 2 ,\
      "<echo '30000 0.025'" notitle lc 0x008B8B pt 6 ps 0.5 ,\
      "<echo '50000 0.025\n90000 0.025'" notitle  with line lc 0x008B8B  lt 3 ,\




if (file_exists("SR_MONOTONIC.STAGNATION.out")){
   stagnationSRMono=`cat SR_MONOTONIC.STAGNATION.out | head -n 1 2>/dev/null`
   set arrow from stagnationSRMono, graph 0 to stagnationSRMono, graph 1 nohead  linecolor "red" dashtype 7 lw 2
}
set label "sr_monotonic"  noenhanced at 1400,0.0125

set output "stagnationErrorLog.pdf"
plot "NEAREST.out"       using 1:4 notitle  lc "dark-green" pt 2 ps 0.8,\
     "RANDOM.0.out"      using 1:4 notitle  lc "dark-red" pt 2 ps 0.8,\
     "RANDOM.post.out"   using 1:3 notitle  lc "dark-red" pt 6 ps 0.5 ,\
     "RANDOM.post.out"   using 1:7 notitle  with line lc "dark-red" lt 3,\
     "AVERAGE.0.out"     using 1:4 notitle  lc "blue" pt 2 ps 0.8,\
     "AVERAGE.post.out"  using 1:3 notitle lc "blue" pt 6  ps 0.5 ,\
     "AVERAGE.post.out"  using 1:7 notitle with line lc "blue" lt 3,\
     "RANDOM_DET.0.out"      using 1:4 notitle  lc "dark-violet" pt 2 ps 0.8,\
     "RANDOM_DET.post.out"   using 1:3 notitle  lc "dark-violet" pt 6 ps 0.5 ,\
     "RANDOM_DET.post.out"   using 1:7 notitle  with line lc "dark-violet" lt 3,\
     "AVERAGE_DET.0.out"     using 1:4 notitle lc 0x008B8B pt 2 ps 0.8,\
     "AVERAGE_DET.post.out"  using 1:3 notitle lc 0x008B8B pt 6  ps 0.5,\
     "AVERAGE_DET.post.out"  using 1:7 notitle  with line lc 0x008B8B  lt 3 ,\
     "SR_MONOTONIC.0.out" using 1:4 notitle lc "red" pt 2 ps 0.8,\
     "SR_MONOTONIC.post.out"  using 1:3 notitle  lc "red" pt 6  ps 0.5,\
     "SR_MONOTONIC.post.out"  using 1:7 notitle  with line lc "red"  lt 3,\
      "<echo '15000 0.40'" notitle lc "dark-green" pt 2 ,\
      "<echo '15000 0.2'" notitle lc "dark-red" pt 2 ,\
      "<echo '30000 0.2'" notitle lc "dark-red" pt 6 ps 0.5 ,\
      "<echo '50000 0.2\n90000 0.2'" notitle  with line lc "dark-red"  lt 3 ,\
      "<echo '15000 0.1'" notitle lc "blue" pt 2 ,\
      "<echo '30000 0.1'" notitle lc "blue" pt 6 ps 0.5 ,\
      "<echo '50000 0.1\n90000 0.1'" notitle  with line lc "blue"  lt 3 ,\
      "<echo '15000 0.05'" notitle lc "dark-violet" pt 2 ,\
      "<echo '30000 0.05'" notitle lc "dark-violet" pt 6 ps 0.5 ,\
      "<echo '50000 0.05\n90000 0.05'" notitle  with line lc "dark-violet"  lt 3 ,\
      "<echo '15000 0.025'" notitle lc 0x008B8B pt 2 ,\
      "<echo '30000 0.025'" notitle lc 0x008B8B pt 6 ps 0.5 ,\
      "<echo '50000 0.025\n90000 0.025'" notitle  with line lc 0x008B8B  lt 3 ,\
      "<echo '15000 0.0125'" notitle lc "red" pt 2 ,\
      "<echo '30000 0.0125'" notitle lc "red" pt 6 ps 0.5 ,\
      "<echo '50000 0.0125\n90000 0.0125'" notitle  with line lc "red"  lt 3 ,\


