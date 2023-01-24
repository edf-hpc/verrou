
set terminal pdf

set logscale x
set logscale y
set yrange [1.e7:1.e9]
set xrange [1.e7:1.e8]
set ylabel "accumulator value"
set xlabel "increment number"

set format y "10^{%L}"
set key top left
#set key at 14000000, 0.0000004
set grid
set size square

set output "stagnationLog-0_st.pdf"
plot "NEAREST_st.out"     using 1:2 title 'nearest' lc "dark-green" pt 3,\
     "NEAREST_st.out"     using 1:5 title 'reference' lc "black"  pt 0 with lines

set output "stagnationLog-1_st.pdf"
plot "NEAREST_st.out"     using 1:2 title 'nearest' lc "dark-green" pt 3,\
     "RANDOM_st.out"     using 1:2 title 'random' lc "dark-red" pt 1 ,\
     "AVERAGE_st.out"     using 1:2 title 'average' lc "blue" pt 1,\
     "NEAREST_st.out"     using 1:5 title 'reference' lc "black"  pt 0 with lines


set output "stagnationLog_st.pdf"
plot "NEAREST_st.out"     using 1:2 title 'nearest' lc "dark-green" pt 3,\
     "RANDOM_st.out"     using 1:2 title 'random' lc "dark-red" pt 1 ,\
     "AVERAGE_st.out"     using 1:2 title 'average' lc "blue" pt 1,\
     "RANDOM_DET_st.out" using 1:2 title 'random\_det' lc "orange-red" pt 12,\
     "AVERAGE_DET_st.out" using 1:2 title 'average\_det' lc 0x008B8B pt 4,\
     "NEAREST_st.out"     using 1:5 title 'reference' lc "black"  pt 0 with lines