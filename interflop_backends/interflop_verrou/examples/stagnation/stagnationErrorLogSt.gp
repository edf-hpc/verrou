
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
#set size square
file_exists(file) = system("[ -f '".file."' ] && echo '1' || echo '0'") + 0

if (file_exists("NEAREST.STAGNATION_st.out")) {
   stagnationNearest=`cat "NEAREST.STAGNATION_st.out"`
    set arrow from stagnationNearest, graph 0 to stagnationNearest, graph 1 nohead  linecolor "dark-green" dashtype 4 lw 2
}

set output "stagnationErrorLog-0_st.pdf"
plot "NEAREST_st.out"     using 1:4 title 'nearest' lc "dark-green" pt 3

set output "stagnationErrorLog-1_st.pdf"
plot "NEAREST_st.out"     using 1:4 title 'nearest' lc "dark-green" pt 3,\
     "RANDOM_st.out"     using 1:4 title 'random' lc "dark-red" pt 1 ,\
     "AVERAGE_st.out"     using 1:4 title 'average' lc "blue" pt 1

if (file_exists("RANDOM_DET.STAGNATION_st.out")){
   stagnationRandomDet=`cat "RANDOM_DET.STAGNATION_st.out" 2>/dev/null`
   set arrow from stagnationRandomDet, graph 0 to stagnationRandomDet, graph 1 nohead  linecolor "orange-red" dashtype 2 lw 2
}

if (file_exists("AVERAGE_DET.STAGNATION_st.out")){
   stagnationAverageDet=`cat "AVERAGE_DET.STAGNATION_st.out" 2>/dev/null`
   set arrow from stagnationAverageDet, graph 0 to stagnationAverageDet, graph 1 nohead  linecolor 0x008B8B dashtype 5 lw 2
}


set output "stagnationErrorLog-2_st.pdf"
plot "NEAREST_st.out"     using 1:4 title 'nearest' lc "dark-green" pt 3,\
     "RANDOM_st.out"     using 1:4 title 'random' lc "dark-red" pt 1 ,\
     "AVERAGE_st.out"     using 1:4 title 'average' lc "blue" pt 1,\
     "RANDOM_DET_st.out" using 1:4 title 'random\_det' lc "orange-red" pt 12,\
     "AVERAGE_DET_st.out" using 1:4 title 'average\_det' lc 0x008B8B pt 4


if (file_exists("SR_MONOTONIC.STAGNATION_st.out")){
   stagnationSRMono=`cat "SR_MONOTONIC.STAGNATION_st.out"  2>/dev/null`
   set arrow from stagnationSRMono, graph 0 to stagnationSRMono, graph 1 nohead  linecolor "red" dashtype 7 lw 2
}

set output "stagnationErrorLog_st.pdf"
plot "NEAREST_st.out"     using 1:4 title 'nearest' lc "dark-green" pt 3,\
     "RANDOM_st.out"     using 1:4 title 'random' lc "dark-red" pt 1 ,\
     "AVERAGE_st.out"     using 1:4 title 'average' lc "blue" pt 1,\
     "RANDOM_DET_st.out" using 1:4 title 'random\_det' lc "orange-red" pt 12,\
     "AVERAGE_DET_st.out" using 1:4 title 'average\_det' lc 0x008B8B pt 4,\
     "SR_MONOTONIC.out" using 1:4 title 'sr\_monotonic' lc "red" pt 6
