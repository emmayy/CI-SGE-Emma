set timestamp "%Y-%m-%d %T"; 
set title "_title_";
set terminal png;
set output "_plot_path_";
set xrange [-10:300];
set yrange [0:30000];
set xlabel "Frame Number";
set ylabel "CPU time(ms)";
#set title "EncodeFrame() performance curve"
set grid;
