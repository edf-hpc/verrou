
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

set output "stagnationErrorLogSt-0.pdf"

plot "NEAREST_st.out"       using 1:4 notitle  lc "dark-green" pt 2 ,\
      "<echo '15000 0.40'" notitle lc "dark-green" pt 2 ,\


set label "random" at	    1400,0.2
set label "average" at      1400,0.1
set label "first" at      12000,0.65 font "sans,10"
set label "mean" at       22000,0.65 font "sans,10"
set label "max" at        50000,0.65 font "sans,10"


set output "stagnationErrorLogSt-1.pdf"
plot "NEAREST_st.out"       using 1:4 notitle  lc "dark-green" pt 2 ps 0.8,\
     "RANDOM_st.0.out"      using 1:4 notitle  lc "dark-red" pt 2 ps 0.8,\
     "RANDOM_st.post.out"   using 1:3 notitle  lc "dark-red" pt 6 ps 0.5 ,\
     "RANDOM_st.post.out"   using 1:7 notitle  with line lc "dark-red" lt 3,\
     "AVERAGE_st.0.out"      using 1:4 notitle  lc "blue" pt 2 ps 0.8,\
     "AVERAGE_st.post.out"   using 1:3 notitle  lc "blue" pt 6 ps 0.5 ,\
     "AVERAGE_st.post.out"   using 1:7 notitle  with line lc "blue" lt 3,\
      "<echo '15000 0.40'" notitle lc "dark-green" pt 2 ,\
      "<echo '15000 0.2'" notitle lc "dark-red" pt 2 ,\
      "<echo '30000 0.2'" notitle lc "dark-red" pt 6 ps 0.5 ,\
      "<echo '50000 0.2\n90000 0.2'" notitle  with line lc "dark-red"  lt 3 ,\
      "<echo '15000 0.1'" notitle lc "blue" pt 2 ,\
      "<echo '30000 0.1'" notitle lc "blue" pt 6 ps 0.5 ,\
      "<echo '50000 0.1\n90000 0.1'" notitle  with line lc "blue"  lt 3 ,\



set label "random\_det" noenhanced at	    1400,0.05
set label "average\_det"  noenhanced  at 1400,0.025

set output "stagnationErrorLogSt-2.pdf"
plot "NEAREST_st.out"       using 1:4 notitle  lc "dark-green" pt 2 ps 0.8,\
     "RANDOM_st.0.out"      using 1:4 notitle  lc "dark-red" pt 2 ps 0.8,\
     "RANDOM_st.post.out"   using 1:3 notitle  lc "dark-red" pt 6 ps 0.5 ,\
     "RANDOM_st.post.out"   using 1:7 notitle  with line lc "dark-red" lt 3,\
     "AVERAGE_st.0.out"     using 1:4 notitle  lc "blue" pt 2 ps 0.8,\
     "AVERAGE_st.post.out"  using 1:3 notitle lc "blue" pt 6  ps 0.5 ,\
     "AVERAGE_st.post.out"  using 1:7 notitle with line lc "blue" lt 3,\
     "RANDOM_DET_st.0.out"      using 1:4 notitle  lc "dark-violet" pt 2 ps 0.8 ,\
     "RANDOM_DET_st.post.out"   using 1:3 notitle  lc "dark-violet" pt 6 ps 0.5 ,\
     "RANDOM_DET_st.post.out"   using 1:7 notitle  with line lc "dark-violet" lt 3,\
     "AVERAGE_DET_st.0.out"     using 1:4 notitle lc 0x008B8B pt 2 ps 0.8,\
     "AVERAGE_DET_st.post.out"  using 1:3 notitle lc 0x008B8B pt 6  ps 0.5,\
     "AVERAGE_DET_st.post.out"  using 1:7 notitle  with line lc 0x008B8B  lt 3 ,\
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


set label "sr_monotonic"  noenhanced at 1400,0.0125

set output "stagnationErrorLogSt.pdf"
plot "NEAREST_st.out"       using 1:4 notitle  lc "dark-green" pt 2 ps 0.8,\
     "RANDOM_st.0.out"      using 1:4 notitle  lc "dark-red" pt 2 ps 0.8,\
     "RANDOM_st.post.out"   using 1:3 notitle  lc "dark-red" pt 6 ps 0.5 ,\
     "RANDOM_st.post.out"   using 1:7 notitle  with line lc "dark-red" lt 3,\
     "AVERAGE_st.0.out"     using 1:4 notitle  lc "blue" pt 2 ps 0.8,\
     "AVERAGE_st.post.out"  using 1:3 notitle lc "blue" pt 6  ps 0.5 ,\
     "AVERAGE_st.post.out"  using 1:7 notitle with line lc "blue" lt 3,\
     "RANDOM_DET_st.0.out"      using 1:4 notitle  lc "dark-violet" pt 2 ps 0.8,\
     "RANDOM_DET_st.post.out"   using 1:3 notitle  lc "dark-violet" pt 6 ps 0.5 ,\
     "RANDOM_DET_st.post.out"   using 1:7 notitle  with line lc "dark-violet" lt 3,\
     "AVERAGE_DET_st.0.out"     using 1:4 notitle lc 0x008B8B pt 2 ps 0.8,\
     "AVERAGE_DET_st.post.out"  using 1:3 notitle lc 0x008B8B pt 6  ps 0.5,\
     "AVERAGE_DET_st.post.out"  using 1:7 notitle  with line lc 0x008B8B  lt 3 ,\
     "SR_MONOTONIC_st.0.out" using 1:4 notitle lc "red" pt 2 ps 0.8,\
     "SR_MONOTONIC_st.post.out"  using 1:3 notitle  lc "red" pt 6  ps 0.5,\
     "SR_MONOTONIC_st.post.out"  using 1:7 notitle  with line lc "red"  lt 3,\
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
      

