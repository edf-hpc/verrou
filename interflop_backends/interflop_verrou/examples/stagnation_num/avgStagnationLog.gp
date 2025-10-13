
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



set label "random" at	    1400,0.2
set label "average" at      1400,0.1
set label "first" at      12000,0.65 font "sans,10"
set label "mean" at       22000,0.65 font "sans,10"
set label "max" at        50000,0.65 font "sans,10"

set label "average\_4" noenhanced at	    1400,0.05
set label "average\_8"  noenhanced  at 1400,0.025
set label "average\_16"  noenhanced at 1400,0.0125
set label "average\_32"  noenhanced at 1400,0.006
set label "average\_64"  noenhanced at 1400,0.003


set output "avgStagnationErrorLog.pdf"
plot "avg_study_NEAREST.out"       using 1:4 notitle  lc "dark-green" pt 2 ps 0.8,\
     "avg_study_RANDOM.0.out"      using 1:4 notitle  lc "dark-red" pt 2 ps 0.8,\
     "RANDOM.post.out"   using 1:3 notitle  lc "dark-red" pt 6 ps 0.5 ,\
     "RANDOM.post.out"   using 1:7 notitle  with line lc "dark-red" lt 3,\
     "avg_study_AVERAGE_1.0.out"     using 1:4 notitle  lc "blue" pt 2 ps 0.8,\
     "AVERAGE_1.post.out"  using 1:3 notitle lc "blue" pt 6  ps 0.5 ,\
     "AVERAGE_1.post.out"  using 1:7 notitle with line lc "blue" lt 3,\
     "avg_study_AVERAGE_4.0.out"      using 1:4 notitle  lc "dark-violet" pt 2 ps 0.8,\
     "AVERAGE_4.post.out"   using 1:3 notitle  lc "dark-violet" pt 6 ps 0.5 ,\
     "AVERAGE_4.post.out"   using 1:7 notitle  with line lc "dark-violet" lt 3,\
     "avg_study_AVERAGE_8.0.out"     using 1:4 notitle lc 0x008B8B pt 2 ps 0.8,\
     "AVERAGE_8.post.out"  using 1:3 notitle lc 0x008B8B pt 6  ps 0.5,\
     "AVERAGE_8.post.out"  using 1:7 notitle  with line lc 0x008B8B  lt 3 ,\
     "avg_study_AVERAGE_16.0.out" using 1:4 notitle lc "red" pt 2 ps 0.8,\
     "AVERAGE_16.post.out"  using 1:3 notitle  lc "red" pt 6  ps 0.5,\
     "AVERAGE_16.post.out"  using 1:7 notitle  with line lc "red"  lt 3,\
     "avg_study_AVERAGE_32.0.out" using 1:4 notitle lc "gray" pt 2 ps 0.8,\
     "AVERAGE_32.post.out"  using 1:3 notitle  lc "gray" pt 6  ps 0.5,\
     "AVERAGE_32.post.out"  using 1:7 notitle  with line lc "gray"  lt 3,\
     "avg_study_AVERAGE_64.0.out" using 1:4 notitle lc "orange" pt 2 ps 0.8,\
     "AVERAGE_64.post.out"  using 1:3 notitle  lc "orange" pt 6  ps 0.5,\
     "AVERAGE_64.post.out"  using 1:7 notitle  with line lc "orange"  lt 3,\
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
      "<echo '15000 0.006'" notitle lc "grey" pt 2 ,\
      "<echo '30000 0.006'" notitle lc "grey" pt 6 ps 0.5 ,\
      "<echo '50000 0.006\n90000 0.006'" notitle  with line lc "grey"  lt 3 ,\
      "<echo '15000 0.003'" notitle lc "orange" pt 2 ,\
      "<echo '30000 0.003'" notitle lc "orange" pt 6 ps 0.5 ,\
      "<echo '50000 0.003\n90000 0.003'" notitle  with line lc "orange"  lt 3 ,\


