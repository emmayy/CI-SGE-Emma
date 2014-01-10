# This is a script that draw a curve of Encode/Decode Frame of CPU performance.
# Version 2.0
# Programmed by Oasis
# Date: 8/19/2009
# This version run on SGE or local host.

# !/bin/sh

# chapter 1 check the input parameters
echo "==================================================================="
echo "The execution of the case is started!"
tf_version=2.0
echo "The version of SGE test performance is ${tf_version}."
echo "==================================================================="
echo ""
###########################################################################
# checking parameter setting file
if [ -z $1 ];
then
	echo "Please input the configure file!"
	exit -1
fi

if ! [ -e $1 ];
then 
	echo "the configure file is not exist!"
	exit -1
fi

echo "maybe need several seconds..."
echo ""
host_name=`hostname`
echo "the hostname for the machine in which job is running is ${host_name}."
echo "-----------------------------------------------------------------------"
############################################################################

source $1		# load the "*.cfg" file parameters

############################################################################


############################################################################
# end of chapter1


# chapter 2 compiling the wels and T27 encoders
############################################################################
# the first session:
# compile the "welsenc.cpp"
echo "-----------------------------------------------------------------------"
echo "Compiling welsenc.cpp(Wels)......"

cd $wels_bin_dir
rm h264enc.exe -f
echo > delta_t.txt

cd $wels_build_dir
make clean
make

wels_time=`date +%k-%M-%S`
wels_tmp="${host_name}_wels_${wels_time}.exe"
wels_name=`echo ${wels_tmp} | tr " " "0"`
delta_wels_tmp="${host_name}_delta_wels_${wels_time}.txt"
delta_wels_name=`echo ${delta_wels_tmp} | tr " " "0"`

cd $wels_bin_dir
mv h264enc.exe $wels_name
#mv delta_t.txt $delta_wels_name

#########################################################################

#########################################################################
# the second session:
# Compile the T27---"h264enc.cpp"
echo "-----------------------------------------------------------------------"
echo "Compiling h264enc.cpp(T27)......"

cd $T27_bin_dir
rm h264enc.exe -f
#rm *.txt -f
echo > delta_t_T27.txt 
 
cd $T27_build_dir
make clean
make

T27_time=`date +%k-%M-%S`
T27_tmp="${host_name}_T27_${T27_time}.exe"
T27_name=`echo ${T27_tmp} | tr " " "0"`
delta_T27_tmp="${host_name}_delta_T27_${T27_time}.txt"
delta_T27_name=`echo ${delta_T27_tmp} | tr " " "0"`

cd $T27_bin_dir
mv h264enc.exe $T27_name
mv delta_t_T27.txt  $delta_T27_name

###################################################################################

###################################################################################
# end of chapter 2

# chapter 3 compiling the wels and T27 decoders
###################################################################################
# the first session:
# compile the "h264dec.cpp" of Wels
echo "-----------------------------------------------------------------------"
echo "Compiling h264dec.cpp(Wels)......"

cd $dec_wels_bin_dir
rm h264dec.exe -f
echo > delta_Wels_dec.txt

cd $dec_wels_build_dir
make clean
make

#########################################################################
# the second session:
# Compile the T27---"h264dec.cpp"
echo "-----------------------------------------------------------------------"
echo "Compiling h264dec.cpp(T27)......"

cd $dec_T27_bin_dir
rm h264dec.exe -f
echo > delta_T27_dec.txt  
 
cd $dec_T27_build_dir
make clean
make

###################################################################################
# end of chapter 3

# chapter 3 create a folder to save the data(*.png, *.txt, *.exe) of case
###################################################################################
# The following is now drawwing a picture of Encode/Decode Frame curve!

# creat a directory to save the data!
echo "======================================================================="
echo "creating work space..."
echo ""

cd $work_dir
echo "work space is : ${work_dir}..."
echo ""
echo "======================================================================="
echo "Now encoding/deconding compiling and execution ..."

for (( i=0; i < ${seq_num}; i++ ))
do
	echo "encoding yuv is: ${sequence_name[${i}]}"

	test_name=`echo ${sequence_name[${i}]} | sed 's/.yuv//'`
	test_name="test_${test_name}"
	foldername="TestPerf"
	time=$(date +%y-%m-%d)_$(date +%k-%M-%S)
	time2=`echo ${time} | tr " " "0"`

	cd $work_dir
	case_data_dir[${i}]="${foldername}_${test_name}_${time2}"

	mkdir "${case_data_dir[${i}]}"
	if ! [ -d "${case_data_dir[${i}]}" ];then
	   echo "create work space fail!!!"
	   echo ""
	   exit -1
	fi
	echo "created work space success!"

        cd ./"${case_data_dir[${i}]}"
	mkdir ./data
	mkdir ./result
	
	cd $work_dir
	cp ./template.gnu ./"${case_data_dir[${i}]}/result"
	cp ./template_diff.gnu ./"${case_data_dir[${i}]}/result"
	cp ./template_dec.gnu ./"${case_data_dir[${i}]}/result"
	cp ./template_dec.gnu ./"${case_data_dir[${i}]}/result"
	cp ./template_diff_dec.gnu ./"${case_data_dir[${i}]}/result"

	echo "-----------------------------------------------------------------------"
	echo "@Wels:"
	echo "-------------------------Encode performance----------------------------"
	# executing encode case in wels
	cd $wels_bin_dir
	#./h264enc.exe wbxavcenc.cfg -org foreman_cif_300.yuv -bf foreman_test.264 -rec forman_test.yuv -cf 1 -iper 320 -frin 30 -sw 352 -sh 288 -lqp 30
	
	./$wels_name wbxavcenc.cfg -org ${sequence_name[${i}]} -bf "${test_name}_Wels.264" -rec wels_test_case.yuv -cf 1 -iper ${sequence_iper[${i}]} -frin ${sequence_frin[${i}]} -sw ${sequence_sw[${i}]} -sh ${sequence_sh[${i}]} -lqp ${sequence_lqp[${i}]}
	
	mv delta_t.txt $delta_wels_name

	# move important data into folder!
	cp ./"${wels_name}" "${work_dir}""${case_data_dir[${i}]}/result"
	cp ./wels_test_case.yuv "${work_dir}""${case_data_dir[${i}]}/data"
	cp ./"${test_name}_Wels.264" "${work_dir}""${case_data_dir[${i}]}/data"
	cp ./$delta_wels_name "${work_dir}""${case_data_dir[${i}]}/result"
	rm ./"${test_name}_Wels.264" -f

	echo "-------------------------Decode performance----------------------------"
	# executing decode case in wels
	cd $dec_wels_bin_dir

	./h264dec.exe "${work_dir}""${case_data_dir[${i}]}/data/${test_name}_Wels.264" "${test_name}_Wels.yuv"

	# move important data into folder!
	cp ./h264dec.exe "${work_dir}""${case_data_dir[${i}]}/result"
	cp ./delta_Wels_dec.txt "${work_dir}""${case_data_dir[${i}]}/result"
	cp ./"${test_name}_Wels.yuv" "${work_dir}""${case_data_dir[${i}]}/data"

	# executing case in T27
	echo "@T27:"
	echo "-------------------------Encode performance----------------------------"
	cd $T27_bin_dir
	
	./$T27_name -i 60 -B 0 -q 26 -F 30 -o test.264 ${sequence_name[${i}]} ${sequence_sw[${i}]}*${sequence_sh[${i}]}

	mv delta_t_T27.txt  $delta_T27_name
	#delta_T27[${i}]=$delta_T27_name
	# move important data into folder!
	cp ./$T27_name "${work_dir}""${case_data_dir[${i}]}/result"
	cp ./$delta_T27_name "${work_dir}""${case_data_dir[${i}]}/result"
	cp ./test.264 "${work_dir}""${case_data_dir[${i}]}/data"
	cd "${work_dir}""${case_data_dir[${i}]}/data"
	mv test.264 "${test_name}_T27.264"
	
	echo "-------------------------Decode performance----------------------------"
	cd $dec_T27_bin_dir

	./h264dec.exe "${work_dir}""${case_data_dir[${i}]}/data/${test_name}_T27.264" "${test_name}_T27.yuv"

	# move important data into folder!
	cp ./h264dec.exe "${work_dir}""${case_data_dir[${i}]}/result"
	cp ./delta_T27_dec.txt "${work_dir}""${case_data_dir[${i}]}/result"
	cp ./"${test_name}_T27.yuv" "${work_dir}""${case_data_dir[${i}]}/data"	
done
###################################################################################
# end of chapter 3
echo -e "Encode/Decode compiling and execution success!\n"

# chapter 4
# plot the Encode/Decode Frame curve
###################################################################################
echo "======================================================================="
echo "plotting the Encode/Decode Frame curve..."
echo "totally, there have ${#case_data_dir[@]} sequences of picture"
echo ""

###################### function: plot the curve of wels and T27  ########################
function plot_separate()
{
	# tell the gnuplot to plot the curve data named "delta_t.txt"
	filename_wels=$delta_wels_name
	filename_T27=$delta_T27_name
	dec_filename_wels="delta_Wels_dec.txt"
	dec_filename_T27="delta_T27_dec.txt"

	if [ -z $filename_wels -a -z $filename_T27 -a -z $dec_filename_wels -a -z $dec_filename_T27 ];
	then
		echo "lack input file data for curve!"
		exit -1
	fi

	# check that the data file is not empty!
	if ! [ -e $filename_wels -a -e $filename_T27 -a -e $dec_filename_wels -a -e $dec_filename_T27 ];
	then 
		echo "the input file does not exist!"
		exit -1
	else
		encodeframe_wels=$filename_wels
		encodeframe_T27=$filename_T27
		dec_decodeframe_Wels=$dec_filename_wels
		dec_decodeframe_T27=$dec_filename_T27
	fi

	# integrate the gnuplot command

	plot_command=`cat template.gnu`

	echo -e "plotted Wels(Encode)...\n"

	png_out_path_Wels="./testPerf_Wels_encode.png"
	plot_name_Wels="EncodeFrame() performance curve(Wels)"
	plot_command_Wels=`echo ${plot_command//_title_/$plot_name_Wels}`
	plot_command_Wels=`echo ${plot_command_Wels//_plot_path_/$png_out_path_Wels}`

	echo $plot_command_Wels >"./encode_tmp_Wels.gnu"
	
	plot_command_Wels=""	
	plot_command_Wels=`echo "$plot_command_Wels plot '${encodeframe_wels}' using 1:2 with lines title 'encodeframe_wels:'"`
	echo $plot_command_Wels >> "./encode_tmp_Wels.gnu"
	gnuplot "./encode_tmp_Wels.gnu"

	echo -e "plotted T27(Encode)...\n"
#======================================================================
	png_out_path_T27="./testPerf_T27_encode.png"
	plot_name_T27="EncodeFrame() performance curve(T27)"
	plot_command_T27=`echo ${plot_command//_title_/$plot_name_T27}`
	plot_command_T27=`echo ${plot_command_T27//_plot_path_/$png_out_path_T27}`

	echo $plot_command_T27 >"./encode_tmp_T27.gnu"

	plot_command_T27=""
	plot_command_T27=`echo "$plot_command_T27 plot '${encodeframe_T27}' using 1:2 with lines title 'encodeframe_T27:'"`
	echo $plot_command_T27 >> "./encode_tmp_T27.gnu"
	gnuplot "./encode_tmp_T27.gnu"

#======================================================================
	dec_plot_command=`cat template_dec.gnu`

	echo -e "plotted Wels(Decode)...\n"

	dec_png_out_path_Wels="./testPerf_Wels_decode.png"
	dec_plot_name_Wels="DecodeFrame() performance curve(Wels)"
	dec_plot_command_Wels=`echo ${dec_plot_command//_title_/$dec_plot_name_Wels}`
	dec_plot_command_Wels=`echo ${dec_plot_command_Wels//_plot_path_/$dec_png_out_path_Wels}`

	echo $dec_plot_command_Wels >"./decode_tmp_Wels.gnu"

	dec_plot_command_Wels=""
	dec_plot_command_Wels=`echo "$dec_plot_command_Wels plot '${dec_decodeframe_Wels}' using 1:2 with lines title 'decodeframe_Wels:'"`
	echo $dec_plot_command_Wels >> "./decode_tmp_Wels.gnu"
	gnuplot "./decode_tmp_Wels.gnu"
#======================================================================
	dec_plot_command=`cat template_dec.gnu`

	echo -e "plotted T27(Decode)...\n"

	dec_png_out_path_T27="./testPerf_T27_decode.png"
	dec_plot_name_T27="DecodeFrame() performance curve(T27)"
	dec_plot_command_T27=`echo ${dec_plot_command//_title_/$dec_plot_name_T27}`
	dec_plot_command_T27=`echo ${dec_plot_command_T27//_plot_path_/$dec_png_out_path_T27}`

	echo $dec_plot_command_T27 >"./decode_tmp_T27.gnu"

	dec_plot_command_T27=""
	dec_plot_command_T27=`echo "$dec_plot_command_T27 plot '${dec_decodeframe_T27}' using 1:2 with lines title 'decodeframe_T27:'"`
	echo $dec_plot_command_T27 >> "./decode_tmp_T27.gnu"
	gnuplot "./decode_tmp_T27.gnu"

	#echo -e "plotted T27...\n"
	#echo "-----------------------------------------------------------------------"
	
	if (($? != 0));
		then
		   echo "error while calling gnuplot, please check ./tmp.gnu"
		   echo "-----------------------------------------------------------------------"
		   exit 1
	fi
}

###################### function: plot the combine of wels and T27(encode) #######################
function plot_combine_enc()
{
	# tell the gnuplot to plot the curve data named "delta_t.txt"
	filename_wels=$delta_wels_name
	filename_T27=$delta_T27_name

	if [ -z $filename_wels -a -z $filename_T27 ];
	then
		echo "lack input file data for curve!"
		exit -1
	fi

	# check that the data file is not empty!
	if ! [ -e $filename_wels -a -e $filename_T27 ];
	then 
		echo "the input file does not exist!"
		exit -1
	else
		encodeframe_wels=$filename_wels
		encodeframe_T27=$filename_T27
	fi

	# integrate the gnuplot command

	plot_command=`cat template.gnu`
	png_out_path="./testPerf_combine_encode.png"
	plot_name="EncodeFrame() performance curve(Wels/T27)"

	plot_command=`echo ${plot_command//_title_/$plot_name}`
	plot_command=`echo ${plot_command//_plot_path_/$png_out_path}`

	echo $plot_command >"./tmp_combine.gnu"

	plot_command=""

	plot_command=`echo "$plot_command plot '${encodeframe_wels}' using 1:2 with lines title 'encodeframe_wels:','${encodeframe_T27}' using 1:2 with lines title 'encodefrmae_T27:'"`
	echo $plot_command >> "./tmp_combine.gnu"
	gnuplot "./tmp_combine.gnu"
	#echo -e "Plotted combine of T27 and Wels...\n"
	if (($? != 0));
		then
		   echo "error while calling gnuplot, please check ./tmp.gnu"
		   echo "-----------------------------------------------------------------------"
		   exit 1
	fi

}

###################### function: plot the diff of wels and T27(encode) #######################
function plot_diff_enc()
{
	#echo "-----------------------------------------------------------------------"
	echo "plot the difference of T27 and Wels EncodeFrame curve..."

	# tell the gnuplot to plot the curve data named "delta_t.txt"
	filename="diff_T27_Wels.txt"

	if [ -z $filename ];
	then
		echo "lack input file data for curve!"
		exit -1
	fi

	# check that the data file is not empty!
	if ! [ -e $filename ];
	then 
		echo "the input file does not exist!"
		exit -1
	else
		encodeframe=$filename
	fi

	# integrate the gnuplot command

	plot_command=`cat template_diff.gnu`
	png_out_path="./testPerf_diff_encode.png"
	plot_name="EncodeFrame() performance curve(Wels/T27)"

	plot_command=`echo ${plot_command//_title_/$plot_name}`
	plot_command=`echo ${plot_command//_plot_path_/$png_out_path}`

	echo $plot_command >"./tmp_diff.gnu"

	plot_command=""

	plot_command=`echo "$plot_command plot '${encodeframe}' using 1:4 with lines title 'encodeframe_wels_and_T27_diff:'"`
		
	echo $plot_command >> "./tmp_diff.gnu"
		
	gnuplot "./tmp_diff.gnu"

	if (($? != 0));
	then
		echo "error while calling gnuplot, please check ./tmp_diff.gnu"
		echo "-----------------------------------------------------"
		exit 1
	fi
}

echo -e "plotting series curves(encode)..."
echo -e "wait for seconds...\n"

###################### function: plot the combine of wels and T27(decode) #######################
function plot_combine_dec()
{
	# tell the gnuplot to plot the curve data named "delta_t.txt"
	filename_wels="delta_T27_dec.txt"
	filename_T27="delta_Wels_dec.txt"

	if [ -z $filename_wels -a -z $filename_T27 ];
	then
		echo "lack input file data for curve!"
		exit -1
	fi

	# check that the data file is not empty!
	if ! [ -e $filename_wels -a -e $filename_T27 ];
	then 
		echo "the input file does not exist!"
		exit -1
	else
		decodeframe_wels=$filename_wels
		decodeframe_T27=$filename_T27
	fi

	# integrate the gnuplot command

	plot_command=`cat template_dec.gnu`
	png_out_path="./testPerf_combine_decode.png"
	plot_name="DecodeFrame() performance curve(Wels/T27)"

	plot_command=`echo ${plot_command//_title_/$plot_name}`
	plot_command=`echo ${plot_command//_plot_path_/$png_out_path}`

	echo $plot_command >"./tmp_combine_dec.gnu"

	plot_command=""

	plot_command=`echo "$plot_command plot '${decodeframe_wels}' using 1:2 with lines title 'decodeframe_wels:','${decodeframe_T27}' using 1:2 with lines title 'decodefrmae_T27:'"`
	echo $plot_command >> "./tmp_combine_dec.gnu"
	gnuplot "./tmp_combine_dec.gnu"

	if (($? != 0));
		then
		   echo "error while calling gnuplot, please check ./tmp_combine_dec.gnu"
		   echo "-----------------------------------------------------------------------"
		   exit 1
	fi

}

###################### function: plot the diff of wels and T27(decode) #######################
function plot_diff_dec()
{
	#echo "-----------------------------------------------------------------------"
	echo "plot the difference of T27 and Wels DecodeFrame curve..."

	# tell the gnuplot to plot the curve data named "delta_t.txt"
	filename="diff_T27_Wels_dec.txt"

	if [ -z $filename ];
	then
		echo "lack input file data for curve!"
		exit -1
	fi

	# check that the data file is not empty!
	if ! [ -e $filename ];
	then 
		echo "the input file does not exist!"
		exit -1
	else
		decodeframe=$filename
	fi

	# integrate the gnuplot command

	plot_command=`cat template_diff_dec.gnu`
	png_out_path="./testPerf_diff_decode.png"
	plot_name="DecodeFrame() performance curve(Wels/T27)"

	plot_command=`echo ${plot_command//_title_/$plot_name}`
	plot_command=`echo ${plot_command//_plot_path_/$png_out_path}`

	echo $plot_command >"./tmp_diff_dec.gnu"

	plot_command=""

	plot_command=`echo "$plot_command plot '${decodeframe}' using 1:4 with lines title 'decodeframe_wels_and_T27_diff:'"`
		
	echo $plot_command >> "./tmp_diff_dec.gnu"
		
	gnuplot "./tmp_diff_dec.gnu"

	if (($? != 0));
	then
		echo "error while calling gnuplot, please check ./tmp_diff_dec.gnu"
		echo "-----------------------------------------------------"
		exit 1
	fi
}

echo -e "plotting series curves(encode/decode)..."
echo -e "wait for seconds...\n"

#############################################################################
##############  plot the curves following as configuration file #############
#############################################################################
for (( i=0; i < ${#case_data_dir[@]}; i++ ))
do
	echo "-----------------------------------------------------------------------"
	cd "${work_dir}""${case_data_dir[${i}]}/result"
	echo "now, plot in "${work_dir}""${case_data_dir[${i}]}"/result"
	echo "the yuv ${sequence_name[${i}]} located in this folder."
	echo "......"
	echo ""
#===================== plot the curve of wels and T27(Encode/decode) =======================
	if ((is_separate_plot == 1));
	then
		echo "plotting T27 and Wels(encode/decode)..."
		plot_separate
		
		echo -e "Have plotted the encode/decode curve of T27 and Wels located in:\n ${work_dir}${case_data_dir[${i}]}/result\n"
		#echo ""
		#echo "-----------------------------------------------------------------------"
	fi
#===================== plot the curve of wels and T27(Encode) =======================
	if ((is_combine_plot_enc == 1));
	then
		echo "plotting combine of T27 and Wels(encode)..."
		plot_combine_enc

		echo -e "Have plotted the encode curve of T27 and Wels combine located in:\n${work_dir}${case_data_dir[${i}]}/result\n"
		#echo ""
		#echo "-----------------------------------------------------------------------"
	fi
#===================== plot the diff of wels and T27(Encode) =======================	
	if ((is_diff_plot_enc == 1));
	then
		# create a pure data source file
		pure_delta_wels="pure_delta_Wels.txt"
		touch $pure_delta_wels
		sed '/#/d' $delta_wels_name | sed '$d' > $pure_delta_wels

		pure_delta_T27="pure_delta_T27.txt"
		touch ${pure_delta_T27}
		sed '/#/d' $delta_T27_name | sed '$d' > $pure_delta_T27
	
		join $pure_delta_T27 $pure_delta_wels > tmp_diff.txt
		awk '{if($3>$2) $4=$3-$2; else $4=$2-$3; print $1,$2,$3,$4}' tmp_diff.txt > diff_T27_Wels.txt 
	
		echo "plotting different of T27 and Wels..."
		plot_diff_enc
		echo -e "Have plotted the diff curve of T27 and Wels located in:\n${work_dir}${case_data_dir[${i}]}/result!\n"

	fi
#===================== plot the curve of wels and T27(Decode) =======================
	if ((is_combine_plot_dec == 1));
	then
		echo "plotting combine of T27 and Wels(encode)..."
		plot_combine_dec

		echo -e "Have plotted the decode curve of T27 and Wels combine located in:\n${work_dir}${case_data_dir[${i}]}/result\n"
	fi
#===================== plot the diff of wels and T27(Decode) =======================	
	if ((is_diff_plot_dec == 1));
	then
		# create a pure data source file
		pure_delta_wels_dec="delta_Wels_dec.txt"
		touch $pure_delta_wels_dec
		sed '/#/d' delta_Wels_dec.txt | sed '$d' > $pure_delta_wels_dec

		pure_delta_T27_dec="delta_T27_dec.txt"
		touch ${pure_delta_T27_dec}
		sed '/#/d' delta_T27_dec.txt | sed '$d' > $pure_delta_T27_dec
	
		join $pure_delta_T27_dec $pure_delta_wels_dec > tmp_diff_dec.txt
		awk '{if($3>$2) $4=$3-$2; else $4=$2-$3; print $1,$2,$3,$4}' tmp_diff_dec.txt > diff_T27_Wels_dec.txt 
	
		echo "plotting different of T27 and Wels..."
		plot_diff_dec
		echo -e "Have plotted the diff curve of T27 and Wels(decode) located in:\n${work_dir}${case_data_dir[${i}]}/result!\n"
	fi
done
		
echo ""
echo "This is the end of encodeframe cpu performance plot process."
echo "-----------------------------------------------------------------------"
for (( i=0; i < ${#case_data_dir[@]}; i++ ))
do
	echo "The case ${sequence_name[${i}]} output data located in direcotry:"${work_dir}""${case_data_dir[${i}]}""
done
echo "-----------------------------------------------------------------------"
#######################################################################################
# end of chapter 4

cd $wels_bin_dir
rm $delta_wels_name -f
rm $wels_name -f

cd $T27_bin_dir
rm $delta_T27_name -f
rm $T27_name -f

echo -e "success!"
echo "-----------------------------------------------------------------------"
#######################################################################################
# end of test performance
