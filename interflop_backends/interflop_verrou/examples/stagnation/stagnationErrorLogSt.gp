
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


set output "stagnationErrorLog-0_st.pdf"
plot "NEAREST_st.out"     using 1:4 title 'nearest' lc "dark-green" pt 3

set output "stagnationErrorLog-1_st.pdf"
plot "NEAREST_st.out"     using 1:4 title 'nearest' lc "dark-green" pt 3,\
     "RANDOM_st.out"     using 1:4 title 'random' lc "dark-red" pt 1 ,\
     "AVERAGE_st.out"     using 1:4 title 'average' lc "blue" pt 1


set output "stagnationErrorLog_st.pdf"
plot "NEAREST_st.out"     using 1:4 title 'nearest' lc "dark-green" pt 3,\
     "RANDOM_st.out"     using 1:4 title 'random' lc "dark-red" pt 1 ,\
     "AVERAGE_st.out"     using 1:4 title 'average' lc "blue" pt 1,\
     "RANDOM_DET_st.out" using 1:4 title 'random\_det' lc "orange-red" pt 12,\
     "AVERAGE_DET_st.out" using 1:4 title 'average\_det' lc 0x008B8B pt 4
