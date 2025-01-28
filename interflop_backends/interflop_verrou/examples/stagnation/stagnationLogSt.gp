
set terminal pdf

set logscale x
set logscale y
#set yrange [1.e7:1.e9]
#set xrange [1.e7:1.e8]
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

#if (file_exists("NEAREST.STAGNATION_st.out")){
#   stagnationNearest=`cat NEAREST.STAGNATION_st.out 2>/dev/null`
#   set arrow from stagnationNearest, graph 0 to stagnationNearest, graph 1 nohead  lineco#lor "dark-green" dashtype 4 lw 2
#}

set output "stagnationLog-0_st.pdf"
plot "NEAREST_st.out"     using 1:5 title 'reference' lc "black"  pt 0 with lines,\
     "NEAREST_st.out"     using 1:2 notitle  lc "dark-green" pt 2 ps 0.8



set output "stagnationLog-1_st.pdf"
plot  "NEAREST_st.out"     using 1:5 title 'reference' lc "black"  pt 0 with lines,\
      "NEAREST_st.out"     using 1:2 notitle lc "dark-green" pt 2 ps 0.8,\
      "RANDOM_st.0.out"    using 1:2 notitle lc "dark-red"   pt 2 ps 0.8,\
      "AVERAGE_st.0.out"   using 1:2 notitle lc "blue"       pt 2 ps 0.8


#if (file_exists("RANDOM_DET.STAGNATION_st.out")) {
#   stagnationRandomDet=`cat RANDOM_DET.STAGNATION_st.out 2>/dev/null`
#   set arrow from stagnationRandomDet, graph 0 to stagnationRandomDet, graph 1 nohead  li#necolor "dark-violet" dashtype 2 lw 2
#}
#if (file_exists("AVERAGE_DET.STAGNATION_st.out")){
#   stagnationAverageDet=`cat AVERAGE_DET.STAGNATION_st.out 2>/dev/null`
#   set arrow from stagnationAverageDet, graph 0 to stagnationAverageDet, graph 1 nohead  #linecolor 0x008B8B dashtype 5 lw 2
#}


set output "stagnationLog-2_st.pdf"
plot "NEAREST_st.out"       using 1:5 title 'reference' lc "black"  pt 0 with lines,\
     "NEAREST_st.out"       using 1:2 notitle lc "dark-green"  pt 2 ps 0.8,\
     "RANDOM_st.0.out"      using 1:2 notitle lc "dark-red"    pt 2 ps 0.8,\
     "AVERAGE_st.0.out"     using 1:2 notitle lc "blue"        pt 2 ps 0.8,\
     "RANDOM_DET_st.0.out"  using 1:2 notitle lc "dark-violet" pt 2 ps 0.8,\
     "AVERAGE_DET_st.0.out" using 1:2 notitle lc 0x008B8B      pt 2 ps 0.8


#if (file_exists("SR_MONOTONIC.STAGNATION_st.0.out")) {
#   stagnationSRMono=`cat SR_MONOTONIC.STAGNATION_st.0.out 2>/dev/null`
#   set arrow from stagnationSRMono, graph 0 to stagnationSRMono, graph 1 nohead  linecolo#r "red" dashtype 7 lw 2
#}

set output "stagnationLog_st.pdf"
plot "NEAREST_st.out"        using 1:5 title 'reference' lc "black"  pt 0 with lines,\
     "NEAREST_st.out"        using 1:2 notitle  lc "dark-green"  pt 2 ps 0.8,\
     "RANDOM_st.0.out"       using 1:2 notitle  lc "dark-red"    pt 2 ps 0.8,\
     "AVERAGE_st.0.out"      using 1:2 notitle  lc "blue"        pt 2 ps 0.8,\
     "RANDOM_DET_st.0.out"   using 1:2 notitle  lc "dark-violet" pt 2 ps 0.8,\
     "AVERAGE_DET_st.0.out"  using 1:2 notitle  lc 0x008B8B      pt 2 ps 0.8,\
     "SR_MONOTONIC_st.0.out" using 1:2 notitle  lc "red"         pt 2 ps 0.8
