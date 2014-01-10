set timestamp "%Y-%m-%d %T"; 
 set title "EncodeFrame() performance curve(Wels/T27)";
 set terminal png;
 set output "./testPerf_output.png";
 set xrange [-10:300]; set yrange [10000:100000]; set xlabel "Frame Number";
 set ylabel "CPU time(ms)"; #set title "EncodeFrame() performance curve"
 set grid;
plot 'delta_t_Wels.txt' using 1:2 with lines title 'encodeframe_wels:','delta_t_T27.txt' using 1:2 with lines title 'encodefrmae_T27:'
