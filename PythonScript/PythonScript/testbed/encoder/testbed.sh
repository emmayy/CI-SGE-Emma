#!/bin/sh


#chapter 1 check the input parameters and run the configure
echo "the execution of the case is started!"
tb_version=2.0
echo "the version of standalone testbed is ${tb_version}"

date
exec_hostname=`hostname`
echo "the machcine to run the case is ${exec_hostname}."

####################################################################################################################		
#checking parameter setting file
if [ -z $1 ]
then
	echo "lack para file"
fi

if ! [ -e $1 ]
then
	echo "the para file dose not exist"
	exit -1
fi

chmod +x $1
###################################################################################################################

####################################################################################################################
#if it's dos file, conver it to unix format

tr -d "\r" < $1 > "${1}_tmp"

#sed 's/^M//' $1 > "${1}_tmp"

cp "${1}_tmp" "${1}" -f

if [ -z $1 ]
then
	echo "the para file is deleted by UFO"
	exit -1
fi

rm "${1}_tmp" -f

#####################################################################################################################

source $1

#####################################################################################################################
# end of chapter 1

#chapter 2 checking para setting
if ((enable_psnr_flag == 0 ))
then
	plot_frame_psnr_flag=0
	plot_rate_psnr_flag=0
fi

if ((enable_vqm_flag == 0 ))
then
	plot_rate_vqm_flag=0
fi

if ((tp_type == 3 ))
then 
	enable_bitrate_flag=0
fi
########################################################################################################################

########################################################################################################################
#some awk function used in the shell
AWKSCRIPT_LOG10=' { printf( "%.2f\n", 4.3429*log($1) ) } ' #compute 10*log10($1)
AWKSCRIPT_DIFF=' { printf( "%.2f\n", (100*($1-$2)/$1) ) } ' #compute ($1-$2)/$1
AWKSCRIPT_ADD=' { printf( "%.2f\n", $1+$2 ) } ' #compute ($1+$2)
AWKSCRIPT_DIVIDE=' { printf( "%.2f\n", $1/$2 ) } ' #compute ($1/$2)
AWKSCRIPT_MULTI=' { printf( "%.4f\n", $1*$2 ) } ' #compute ($1*$2)

########################################################################################################################
#some predfined parameters
TimingFormat[10]="THIRD_NTSC"
TimingFormat[15]="HALF_NTSC"
TimingFormat[25]="PAL"
TimingFormat[30]="NTSC"

#PSNR module
PSNR_EVALUATION="./ComputePSNR.exe"
VQM_EVALUATION="./vqm.exe"
COMPLEXITY_EVALUATION=""
SSIM_EVALUATION="./ssim.exe"

E_PARA_ERROR=65
########################################################################################################################

########################################################################################################################
#end of chapter 2

#chapter 3 function for module calling
current_module=""
CallModule () #$encoder $input_path_bas $output_path  ${testpara}  ${seq_width_bas[index]} ${seq_height_bas[index]} $input_path_el1  ${seq_width_el1[index]} ${seq_height_el1[index]} $input_path_el2 ${seq_width_el2[index]} ${seq_height_el2[index]} ${seq_bitrate[index]} ${seq_fps[index]}		 
{
	if [ -z "$1" ] || [ -z "$2" ] ||  [ -z "$3" ] || [ -z "$4" ] || [ -z "$5" ]
	then 
		echo "parameter error for CallModule()"
		return $E_PARA_ERROR
	fi

	#if[[ $current_module == "encoder1" ]]
	#then
	#echo "input parameters:" 
	#echo "$1,  $2,  $3,  $4,  $5,  $6,  $7,  $8,  $9,  ${10},  ${11},  ${12}" 
	#fi
	
	module_name=`echo $current_module`
	module_name=`echo ${module_name//_INPUT_PATH_BAS/$1}`
	module_name=`echo ${module_name//_OUTPUT_PATH/$2}`
	module_name=`echo ${module_name//_TEST_POINTS/$3}`
	module_name=`echo ${module_name//_FRAME_WIDTH_BAS/$4}`
	module_name=`echo ${module_name//_FRAME_HEIGHT_BAS/$5}`
	#module_name=`echo ${module_name//_INPUT_PATH_EL1/$6}`	
	#module_name=`echo ${module_name//_FRAME_WIDTH_EL1/$7}`
	#module_name=`echo ${module_name//_FRAME_HEIGHT_EL1/$8}`
	#module_name=`echo ${module_name//_INPUT_PATH_EL2/$9}`	
	#module_name=`echo ${module_name//_FRAME_WIDTH_EL2/${10}}`
	#module_name=`echo ${module_name//_FRAME_HEIGHT_EL2/${11}}`
	
	
  if [ ! -z $6 ]
  then
		module_name=`echo ${module_name//_INPUT_PATH_EL1/$6}`	
  fi
  
  if [ ! -z $7 ]
  then
		module_name=`echo ${module_name//_FRAME_WIDTH_EL1/$7}`	
  fi
  
  if [ ! -z $8 ]
  then
		module_name=`echo ${module_name//_FRAME_HEIGHT_EL1/$8}`	
  fi
  
  if [ ! -z $9 ]
  then
		module_name=`echo ${module_name//_INPUT_PATH_EL2/$9}`	
  fi
  
  if [ ! -z ${10} ]
  then
 		module_name=`echo ${module_name//_FRAME_WIDTH_EL2/${10}}`	
  fi
  
  if [ ! -z ${11} ]
  then
 		module_name=`echo ${module_name//_FRAME_HEIGHT_EL2/${11}}`	
  fi


  if [ ! -z ${12} ]
  then
		module_name=`echo ${module_name//_EXTRA_TP_PARA/${12}}`	
  fi

	
	echo "$module_name"
	$module_name	
	
	if (($? != 0))
	then
		echo "error while executing the last command"
		$module_name
		exit 1
	fi
}

tmp_file=""
PsnrEvaluation_LossRatio () #compute psnr_bitrate of the sequence $input_path $post_output_path $encoder_output_path $psnr_file $avsnr_file $seq_width $seq_height $tp_value[n]
{

	if [ -z "$1" ] || [ -z "$2" ] ||  [ -z "$3" ] || [ -z "$4" ] || [ -z "$5" ] || [ -z "$6" ] || [ -z "$7" ] 
	then 
		echo "parameter error for PsnrEvaluation()"
		return $E_PARA_ERROR
	fi
	
	echo "${PSNR_EVALUATION} $1 $2 $4 ${tmp_file} ${6} ${7} "

	$PSNR_EVALUATION $1 $2 $4 ${tmp_file} ${6} ${7} 
	
	avsnr=`cat ${tmp_file}`
	echo  $8 ${avsnr} >> $5
	echo "" > ${tmp_file}  
}

#end of chapter 3


#chapter 4 create working directory for current test case

testbed_hostname=`hostname`
echo "the hostname for the machine in which job is running is ${testbed_hostname}."

time=$(date +%m-%d-%y)_$(date +%k-%M-%S)
time2=`echo ${time}|tr " " "0"`

echo $case_name
case_name=`echo ${case_name//" "/"_"}`
echo $case_name

case_data_directory="${test_data_directory}${case_name}_${time2}"
mkdir $case_data_directory
case_data_directory="${case_data_directory}/data"

echo "test data will be stored in: $case_data_directory"
mkdir $case_data_directory

if ! [ -d ${case_data_directory} ]
then
	echo "can not create directory $case_name"
	exit -1
fi

tmp_gnu_file="${case_data_directory}/tmp.gnu"
tmp_cntl_file="${case_data_directory}/tmp.cntl"


case_result_directory="${test_result_directory}${case_name}_${time2}"
case_result_directory="${case_result_directory}/result"

echo "test result will be stored in: $case_result_directory"
if ! [ -d ${case_result_directory} ]
then
	mkdir $case_result_directory
fi

if ! [ -d ${case_result_directory} ]
then
	echo "can not create directory $case_result_directory"
	exit -1
fi


test_data_path="$case_result_directory/test_report.txt"

echo "case name: $case_name " > $test_data_path

for ((index=1; index <= seq_num ; index++))
do
       
        if [[ -z ${seq_description[index]} ]]
        
        then
        
        if (( seq_slayer_num[index] == 1 ))
        	then
		        seq_description[index]=${seq_name_bas[index]}
		        if [[ ! -z ${seq_fps_bas[index]} ]]
		        then
		        	seq_description[index]=`echo "${seq_description[index]} ${seq_fps_bas[index]} fps"`
		        fi
		
		        if [[ ! -z ${seq_bitrate_bas[index]} ]]
		        then
		        	seq_description[index]=`echo "${seq_description[index]} ${seq_bitrate_bas[index]} bps"`
		  	fi
	        fi
	if (( seq_slayer_num[index] == 2 ))
        	then
		        seq_description[index]=${seq_name_el1[index]}
		        if [[ ! -z ${seq_fps_el1[index]} ]]
		        then
		        	seq_description[index]=`echo "${seq_description[index]} ${seq_fps_el1[index]} fps"`
		        fi
		
		        if [[ ! -z ${seq_bitrate_el1[index]} ]]
		        then
		        	seq_description[index]=`echo "${seq_description[index]} ${seq_bitrate_el1[index]} bps"`
		        fi
	        fi
	 if (( seq_slayer_num[index] == 3 ))
        	then
		        seq_description[index]=${seq_name_el2[index]}
		        if [[ ! -z ${seq_fps_el2[index]} ]]
		        then
		        	seq_description[index]=`echo "${seq_description[index]} ${seq_fps_el2[index]} fps"`
		        fi
		
		        if [[ ! -z ${seq_bitrate_el2[index]} ]]
		        then
		        	seq_description[index]=`echo "${seq_description[index]} ${seq_bitrate_el2[index]} bps"`
		        fi
	        fi

	fi
        
	echo "sequence $index: ${seq_description[index]}" >> $test_data_path

	cp "./${seq_name_bas[index]}.yuv" "${case_data_directory}/" -f
	
	if (( seq_slayer_num[index] == 2 ))
	then
	  cp "./${seq_name_el1[index]}.yuv" "${case_data_directory}/" -f
	fi
	
	if (( seq_slayer_num[index] == 3 ))
	  then
	  cp "./${seq_name_el1[index]}.yuv" "${case_data_directory}/" -f
	  cp "./${seq_name_el2[index]}.yuv" "${case_data_directory}/" -f
	fi
	
#end of chapter 4 
  
#chapter 5 prepare the uyvy data for VQM
	echo "test ${seq_description[index]}"
	
	input_path0="${case_data_directory}/${seq_name_bas[index]}.yuv"	
	tmp_file="${case_result_directory}/${seq_name_bas[index]}_tmp.txt"
	
	tmp_vqm="${case_data_directory}/${seq_name_bas[index]}.uyvy"
	
	original_seq_vqm0=$tmp_vqm
	original_seq=${input_path0}

	if(( enable_vqm_flag == 1))
	then
		echo "./yuvconvert.exe $original_seq $original_seq_vqm0 ${seq_width_bas[index]} ${seq_height_bas[index]} 0"
		./yuvconvert.exe $original_seq $original_seq_vqm0 ${seq_width_bas[index]} ${seq_height_bas[index]} 0
	fi

	if (( tp_type == 3 ))
	then
		original_seq="${case_data_directory}/${seq_name_bas[index]}_long.yuv"	
		original_seq_vqm0="${case_data_directory}/${seq_name_bas[index]}_long.uyvy"	
		rm $original_seq -f
		
		for((rpt=1; rpt <= ${seq_loops_bas[index]}; rpt++))
		do
			cat "$input_path0" >> $original_seq
			cat "$tmp_vqm" >> $original_seq_vqm0
		done	
	fi

	
	if(( seq_slayer_num[index] >= 2 ))
	then
	input_path1="${case_data_directory}/${seq_name_el1[index]}.yuv"	
	tmp_file="${case_result_directory}/${seq_name_el1[index]}_tmp.txt"
	
	tmp_vqm="${case_data_directory}/${seq_name_el1[index]}.uyvy"
	
	original_seq_vqm1=$tmp_vqm
	original_seq=${input_path1}
	
	if(( enable_vqm_flag == 1))
	then
	echo "./yuvconvert.exe $original_seq $original_seq_vqm1 ${seq_width_el1[index]} ${seq_height_el1[index]} 0"
	./yuvconvert.exe $original_seq $original_seq_vqm1 ${seq_width_el1[index]} ${seq_height_el1[index]} 0
	fi	
	
		if (( tp_type == 3 ))
		then
			original_seq="${case_data_directory}/${seq_name_el1[index]}_long.yuv"	
			original_seq_vqm1="${case_data_directory}/${seq_name_el1[index]}_long.uyvy"	
			rm $original_seq -f
			
			for((rpt=1; rpt <= ${seq_loops_el1[index]}; rpt++))
			do
				cat "$input_path1" >> $original_seq
				cat "$tmp_vqm" >> $original_seq_vqm1
			done	
		fi
	fi
	
	if(( seq_slayer_num[index] == 3 ))
	then
	input_path2="${case_data_directory}/${seq_name_el2[index]}.yuv"	
	tmp_file="${case_result_directory}/${seq_name_el2[index]}_tmp.txt"
	
	tmp_vqm="${case_data_directory}/${seq_name_el2[index]}.uyvy"
	
	original_seq_vqm2=$tmp_vqm
	original_seq=${input_path2}

	if(( enable_vqm_flag == 1))
	then
		echo "./yuvconvert.exe $original_seq $original_seq_vqm2 ${seq_width_el2[index]} ${seq_height_el2[index]} 0"
		./yuvconvert.exe $original_seq $original_seq_vqm2 ${seq_width_el2[index]} ${seq_height_el2[index]} 0
	fi
	
	
		if (( tp_type == 3 ))
		then
			original_seq="${case_data_directory}/${seq_name_el2[index]}_long.yuv"	
			original_seq_vqm2="${case_data_directory}/${seq_name_el2[index]}_long.uyvy"	
			rm $original_seq -f
			for((rpt=1; rpt <= ${seq_loops_el2[index]}; rpt++))
			do
				cat "$input_path2" >> $original_seq
				cat "$tmp_vqm" >> $original_seq_vqm2
			done	
		fi

	fi

# end of chapter 5

#chapter 6 run the procedures for this case according to the test points
  	original_backup=0
	for ((j=1; j <= tp_num; j++))	
	do
		testpara=${tp_para[j]}
		extra_test_para=${extra_tp_para[j]}
		if [[ $testpara = "" ]]
		then
			testpara=${tp_value[j]}
			if (( tp_type == 2 ))
			then
			let "testpara /= 1000"
			fi
		fi

		echo "testing testpoints: $testpara"
		
		seq_name[index]=""
		seq_width[index]=""
		seq_height[index]=""
		seq_fps[index]=""			
		seq_bitrate[index]=""
		seq_loops[index]=""
		
		if(( start_point_codec != 1 ))
		then
		n2=$start_point_codec
		else
		n2=1
		fi
	
	
		if(( end_point_codec != codec_num ))
		then
		n3=$end_point_codec
		else
		n3=$codec_num
		fi
		
		for ((n=$n2; n <= $n3; n++))
		do
		
		if(( original_backup == 1 ))
		then
			mv ${original_seq}.org ${original_seq}
			original_backup=0
		fi
			
				
		  if(( codec_target_slayer[n]==0 ))
		  then
		      	original_seq_size=`wc -c $original_seq_vqm0 | awk '{print $1}'`
		      	echo "original_seq_size=$original_seq_size"
		      	seq_name[index]=${seq_name_bas[index]}
		      	seq_width[index]=${seq_width_bas[index]}
			seq_height[index]=${seq_height_bas[index]}
			seq_fps[index]=${seq_fps_bas[index]}			
			seq_bitrate[index]=${seq_bitrate_bas[index]}
			seq_loops[index]=${seq_loops_bas[index]}
		      	input_path=${input_path0}
		      	original_seq=${input_path0}
		      	original_seq_vqm=${original_seq_vqm0}
		      	if (( tp_type == 3 ))
				then
				original_seq="${case_data_directory}/${seq_name_bas[index]}_long.yuv"
				fi	
			fi
			
		if(( codec_target_slayer[n]==1 ))
		  then
	      		original_seq_size=`wc -c $original_seq_vqm1 | awk '{print $1}'`
	      		echo "original_seq_size=$original_seq_size"
	      		seq_name[index]=${seq_name_el1[index]}
	      		seq_width[index]=${seq_width_el1[index]}
			seq_height[index]=${seq_height_el1[index]}
			seq_fps[index]=${seq_fps_el1[index]}			
			seq_bitrate[index]=${seq_bitrate_el1[index]}
			seq_loops[index]=${seq_loops_el1[index]}
	      		input_path=${input_path1}
	      		original_seq=${input_path1}
	      		original_seq_vqm=${original_seq_vqm1}
	      		if (( tp_type == 3 ))
				then
				original_seq="${case_data_directory}/${seq_name_el1[index]}_long.yuv"
				fi
			fi
			
		if(( codec_target_slayer[n]==2 ))
		  then
	      		original_seq_size=`wc -c $original_seq_vqm2 | awk '{print $1}'`
	      		echo "original_seq_size=$original_seq_size"
	      		seq_name[index]=${seq_name_el2[index]}
	      		seq_width[index]=${seq_width_el2[index]}
			seq_height[index]=${seq_height_el2[index]}
			seq_fps[index]=${seq_fps_el2[index]}			
			seq_bitrate[index]=${seq_bitrate_el2[index]}
			seq_loops[index]=${seq_loops_el2[index]}
	      		input_path=${input_path2}
	      		original_seq=${input_path2}
	      		original_seq_vqm=${original_seq_vqm2}
	      		if (( tp_type == 3 ))
				then
				original_seq="${case_data_directory}/${seq_name_el2[index]}_long.yuv"
				fi
			fi
			
	avsnr_file[n]="${case_result_directory}/${seq_name[index]}_${n}_avnr.txt"
	
	#test_point for network mode only exist at netsimulator part
	
			if [[ ${preprocessor[n]} = "" ]]
			then
			 	preprocessor_output_path=$input_path0
			else
			 	preprocessor_output_path="${case_data_directory}/${seq_name[index]}_${n}_preprocessor.yuv"
			 	current_module={preprocessor[n]} 
			 	CallModule $input_path0 $preprocessor_output_path "${testpara}" "${seq_width[index]}" "${seq_height[index]}" "${seq_fps[index]}" "${seq_bitrate[index]}"	"$extra_test_para" 
			fi				
			
			encoder_output_path="${case_data_directory}/${seq_name[index]}_${tp_value[j]}_${n}_raw.264"
			encoder_rec_orig_path="./temp_rec.yuv"
			encoder_rec_output_path="${case_data_directory}/${seq_name[index]}_${tp_value[j]}_${n}_rec.yuv"
			
			encoder_perf_orig_path="./cpu_load.txt"
			encoder_perf_output_path="${case_data_directory}/${seq_name[index]}_${tp_value[j]}_${n}_perf.txt"
			
			if [[  ${encoder[n]} = "" ]]
			then 

		#  for Wels SVC test 			
				if (( same_encoder_flag == 1 && n > n2 )); then
				
					encoder_output_path="${case_data_directory}/${seq_name[index]}_${tp_value[j]}_${n2}_raw.264"
					encoder_rec_output_path="${case_data_directory}/${seq_name[index]}_${tp_value[j]}_${n2}_rec.yuv"
					encoder_perf_output_path="${case_data_directory}/${seq_name[index]}_${tp_value[j]}_${n2}_perf.txt"
					
				else 
					encoder_output_path=$preprocessor_output_path
				fi
				
			else
				encoder_input_path=$preprocessor_output_path
				current_module=${encoder[n]} 
				
				echo "ready to encode"
				if ((seq_slayer_num[index]==3))
		     		then
					CallModule $encoder_input_path $encoder_output_path "${testpara}" "${seq_width_bas[index]}" "${seq_height_bas[index]}" "${input_path1}" "${seq_width_el1[index]}" "${seq_height_el1[index]}" "${input_path2}" "${seq_width_el2[index]}" "${seq_height_el2[index]}"
				fi
					
				if ((seq_slayer_num[index]==2))
		      		then
					CallModule $encoder_input_path $encoder_output_path "${testpara}" "${seq_width_bas[index]}" "${seq_height_bas[index]}" "${input_path1}" "${seq_width_el1[index]}" "${seq_height_el1[index]}"
				fi
					
				if ((seq_slayer_num[index]==1))
		      		then
					CallModule $encoder_input_path $encoder_output_path "${testpara}" "${seq_width_bas[index]}" "${seq_height_bas[index]}"
				fi
				
				if [ -e $encoder_rec_orig_path ] ; then
				mv $encoder_rec_orig_path $encoder_rec_output_path
				else
				echo "the orignal reconstructive doesn't exist!"
				fi
				
				if (( enable_perf_flag == 1 ))
				then
					if [ -e $encoder_perf_orig_path ] ; then
					mv -f $encoder_perf_orig_path $encoder_perf_output_path
					else
					echo "warning: the orignal performance(CPU load) file doesn't exist!"
					fi
				fi
				
			fi				
			
			if (( tp_type == 3 ))
			then
				if [[ ! -z ${seq_loops[index]} ]]
				then
					cp $encoder_output_path "${encoder_output_path}_bak" -f
			
					for((rpt=2; rpt <= ${seq_loops[index]}; rpt++))
					do
						cat "${encoder_output_path}_bak" >> $encoder_output_path
					done	
					rm "${encoder_output_path}_bak" -f		
				fi
			fi
			
			if [[ ${packer[n]} = "" ]]
			then
				packer_output_path=$encoder_output_path
			else
				packer_input_path=$encoder_output_path
				packer_output_path="${case_data_directory}/${seq_name[index]}_${tp_value[j]}_${n}_raw.rtp"
				current_module=${packer[n]} 
#				CallModule $packer_input_path $packer_output_path ${testpara} ${seq_width[index]} ${seq_height[index]}  ${seq_fps[index]} ${seq_bitrate[index]}		 					
				CallModule $packer_input_path $packer_output_path "${testpara}" "${seq_width[index]}" "${seq_height[index]}" "${seq_fps[index]}" "${seq_bitrate[index]}" "$extra_test_para" 	 					
			fi
							
			if [[ ${nsimulator[n]} = "" ]]
			then
				nsimulator_output_path=$packer_output_path
			else
				nsimulator_input_path=$packer_output_path
				nsimulator_output_path="${case_data_directory}/${seq_name[index]}_${tp_value[j]}_${n}.rtp"
				current_module=${nsimulator[n]} 				
#				CallModule $nsimulator_input_path $nsimulator_output_path ${testpara} ${seq_width[index]} ${seq_height[index]}  ${seq_fps[index]} ${seq_bitrate[index]}		 					
				CallModule $nsimulator_input_path $nsimulator_output_path "${testpara}" "${seq_width[index]}" "${seq_height[index]}" "${seq_fps[index]}" "${seq_bitrate[index]}" "$extra_test_para"  					
			fi	
			
			if [[ ${unpacker[n]} = "" ]]
			then
				unpacker_output_path=$nsimulator_output_path
			else
				unpacker_input_path=$nsimulator_output_path
				unpacker_output_path="${case_data_directory}/${seq_name[index]}_${tp_value[j]}_${n}.264"
				current_module=${unpacker[n]} 
#				CallModule $unpacker_input_path $unpacker_output_path ${testpara} ${seq_width[index]} ${seq_height[index]}  ${seq_fps[index]} ${seq_bitrate[index]}		 					
				CallModule $unpacker_input_path $unpacker_output_path "${testpara}" "${seq_width[index]}" "${seq_height[index]}" "${seq_fps[index]}" "${seq_bitrate[index]}" "$extra_test_para" 	 					
			fi		
			
			if [[ ${bextractor[n]} = "" ]]
			then
				extractor_output_path=$unpacker_output_path
			else
				extractor_input_path=$unpacker_output_path
			 	extractor_output_path="${case_data_directory}/${seq_name[index]}_${tp_value[j]}_${n}_extra.264"
			 	extractor_output_path2="${case_data_directory}/${seq_name[index]}_${tp_value[j]}_${n}_extra_2.264"
				extractor_output_path3="${case_data_directory}/${seq_name[index]}_${tp_value[j]}_${n}_extra_3.264"			 	
				current_module=${bextractor[n]} 

			#spatial extraction
			echo ${bextractor[n]} $extractor_input_path $extractor_output_path -l ${codec_target_slayer[n]}
			if (( codec_target_slayer[n] == 0 ))
			then
				${bextractor[n]} $extractor_input_path $extractor_output_path -l 0
			fi
			if (( codec_target_slayer[n] == 1 ))
			then
				${bextractor[n]} $extractor_input_path $extractor_output_path -l 1
			fi
				
			if (( codec_target_slayer[n] == 2 ))
			then
				${bextractor[n]} $extractor_input_path $extractor_output_path -l 2
			fi
				
			#temporal extratction
			echo ${bextractor[n]} -pt trace $extractor_output_path $extractor_output_path2 -t ${codec_target_tlayer[n]}
			if (( codec_target_tlayer[n] == 0 ))
			then
				${bextractor[n]} -pt trace $extractor_output_path $extractor_output_path2 -t 0
				cp -f $extractor_output_path2 $extractor_output_path
				rm -f $extractor_output_path2
#				let "seq_fps[index] /= 8"
				seq_fps[index]=`echo ${seq_fps[index]} 8 | awk "$AWKSCRIPT_DIVIDE"`
				#echo "${seq_fps[index]}/2.0" | bc
				echo "seq_fps[${index}]=${seq_fps[index]}"
			fi
			if (( codec_target_tlayer[n] == 1 ))
			then
				${bextractor[n]} -pt trace $extractor_output_path $extractor_output_path2 -t 1
				cp -f $extractor_output_path2 $extractor_output_path
				rm -f $extractor_output_path2
#				let "seq_fps[index] /= 4"
				seq_fps[index]=`echo ${seq_fps[index]} 4 | awk "$AWKSCRIPT_DIVIDE"`
				echo "seq_fps[${index}]=${seq_fps[index]}"
			fi
				
			if (( codec_target_tlayer[n] == 2 ))
			then
				${bextractor[n]} -pt trace $extractor_output_path $extractor_output_path2 -t 2
				cp -f $extractor_output_path2 $extractor_output_path
				rm -f $extractor_output_path2
#				let "seq_fps[index] /= 2"
				seq_fps[index]=`echo ${seq_fps[index]} 2 | awk "$AWKSCRIPT_DIVIDE"`
				echo "seq_fps[${index}]=${seq_fps[index]}"
			fi
				
			if (( codec_target_tlayer[n] == 3 ))
			then
				${bextractor[n]} -pt trace $extractor_output_path $extractor_output_path2 -t 3
				cp -f $extractor_output_path2 $extractor_output_path
				rm -f $extractor_output_path2
				echo "seq_fps[${index}]=${seq_fps[index]}"
			fi
			
			mv -f "./frametype.txt" ${case_data_directory}/frametype.txt			
			
			#quality extratction
			echo ${bextractor[n]} $extractor_output_path $extractor_output_path3 -q ${codec_target_qlayer[n]}
			if (( codec_target_qlayer[n] == 0 ))
			then
				${bextractor[n]} $extractor_output_path $extractor_output_path3 -f 0
				cp -f $extractor_output_path3 $extractor_output_path
				rm -f $extractor_output_path3
			fi
			if (( codec_target_qlayer[n] == 1 ))
			then
				${bextractor[n]} $extractor_output_path $extractor_output_path3 -f 1
				cp -f $extractor_output_path3 $extractor_output_path
				rm -f $extractor_output_path3
			fi
				
			if (( codec_target_qlayer[n] == 2 ))
			then
				${bextractor[n]} $extractor_output_path $extractor_output_path3 -f 2
				cp -f $extractor_output_path3 $extractor_output_path
				rm -f $extractor_output_path3
			fi
			
			if (( codec_target_qlayer[n] == 3 ))
			then
				${bextractor[n]} $extractor_output_path $extractor_output_path3 -f 3
				cp -f $extractor_output_path3 $extractor_output_path
				rm -f $extractor_output_path3
			fi
			
			#end quality extratction
			fi
			
			
			#start decoding process
			if [[ ${decoder[n]} = "" ]]
			then
				decoder_output_path=$extractor_output_path
			else
				decoder_input_path=$extractor_output_path
			 	decoder_output_path="${case_data_directory}/${seq_name[index]}_${tp_value[j]}_${n}.yuv"
				current_module=${decoder[n]} 

				CallModule $decoder_input_path $decoder_output_path "${testpara}" "${seq_width[index]}" "${seq_height[index]}" "${seq_fps[index]}" "${seq_bitrate[index]}" "$extra_test_para" 	 					
			fi			
			
			if (( tp_type == 3 ))
			then
				decoder_output_path2="${case_data_directory}/${seq_name[index]}_${tp_value[j]}_${n}_recover.yuv"			
				echo ./postprocessor.exe  $decoder_output_path  ${decoder_output_path2} "${unpacker_output_path}_lf" ${seq_width[index]} ${seq_height[index]} $original_seq
				./postprocessor.exe  $decoder_output_path  ${decoder_output_path2} "${unpacker_output_path}_lf" ${seq_width[index]} ${seq_height[index]} $original_seq
				if (( delete_medium_yuv_flag == 1 ))
				then
					rm $decoder_output_path -f
				fi
				decoder_output_path=$decoder_output_path2
			fi
				
			if [[ ${postprocessor[n]} = "" ]]
			then
				post_output_path=$decoder_output_path
			else
				post_input_path=$decoder_output_path
				post_output_path="${case_data_directory}/${seq_name[index]}_${tp_value[j]}_${n}_post.yuv"
				current_module=${postprocessor[n]}

				CallModule $post_input_path $post_output_path "${testpara}" "${seq_width[index]}" "${seq_height[index]}" "${seq_fps[index]}" "${seq_bitrate[index]}" "$extra_test_para"  
			fi	
			#end decoding process
#the end of chapter 6


#chapter 7		start the analysis of PSNR and SSIM
	
			bitrate_value=""
			psnr_value=""
			ssim_value=""
			vqm_value="null"
			vqm_score="null"
			vqm_blurring="null"
			vqm_distortion="null"
			vqm_jerky="null"

			#compute frame number
			yuv_size=`wc -c $post_output_path | awk '{print $1}'`
			frame_size=`echo ${seq_width[index]} ${seq_height[index]} | awk "$AWKSCRIPT_MULTI"`
			echo "frame_size=$frame_size"
			frame_size=`echo ${frame_size} 1.5 | awk "$AWKSCRIPT_MULTI"`
			frame_num=`echo ${yuv_size} $frame_size | awk "$AWKSCRIPT_DIVIDE"`
			
			if (( enable_bitrate_flag == 1 ))
			then
				bitstream_size=`wc -c $extractor_output_path | awk '{print $1}'`
				bitrate_value=`echo ${bitstream_size} $frame_num | awk "$AWKSCRIPT_DIVIDE"`
				bitrate_value=`echo ${bitrate_value} 8 | awk "$AWKSCRIPT_MULTI"`
				bitrate_value=`echo ${bitrate_value} ${seq_fps[index]} | awk "$AWKSCRIPT_MULTI"`
				bitrate_value=`echo ${bitrate_value} 1000 | awk "$AWKSCRIPT_DIVIDE"`
				
				echo "yuv_size=$yuv_size frame_size=$frame_size frame_num=$frame_num bitrate_value=$bitrate_value"
			fi
			
			#adjust the original sequence according to temporal layers target 
			
			original_seq2=${original_seq}_2			
			if (( codec_target_tlayer[n] == 0 ))
			then
 				if [[ ${framespliter[n]} != "" ]]
					then
					echo ${framespliter[n]} $original_seq ${case_data_directory}/frametype.txt $original_seq2 8 "${seq_width[index]}" "${seq_height[index]}"
					${framespliter[n]} $original_seq ${case_data_directory}/frametype.txt $original_seq2 8 "${seq_width[index]}" "${seq_height[index]}"
					return_code=$?
					echo "split frames=$return_code"
					
					mv ${original_seq} ${original_seq}.org
					original_backup=1				
					cp -f ${original_seq2} ${original_seq}				
					echo "./yuvconvert.exe $original_seq $original_seq_vqm ${seq_width[index]} ${seq_height[index]} 0"
					./yuvconvert.exe $original_seq $original_seq_vqm ${seq_width[index]} ${seq_height[index]} 0
					original_seq_size=`wc -c $original_seq_vqm | awk '{print $1}'`		
					fi						
								
			fi
			
			if (( codec_target_tlayer[n] == 1 ))
			then
				if [[ ${framespliter[n]} != "" ]]
				then
					echo ${framespliter[n]} $original_seq ${case_data_directory}/frametype.txt $original_seq2 4 "${seq_width[index]}" "${seq_height[index]}"
					${framespliter[n]} $original_seq ${case_data_directory}/frametype.txt $original_seq2 4 "${seq_width[index]}" "${seq_height[index]}"
					return_code=$?
					echo "split frames=$return_code"
					
					mv ${original_seq} ${original_seq}.org				
					original_backup=1				
					cp -f ${original_seq2} ${original_seq}				
					echo "./yuvconvert.exe $original_seq $original_seq_vqm ${seq_width[index]} ${seq_height[index]} 0"
					./yuvconvert.exe $original_seq $original_seq_vqm ${seq_width[index]} ${seq_height[index]} 0
					original_seq_size=`wc -c $original_seq_vqm | awk '{print $1}'`
				fi
			fi
			
			if (( codec_target_tlayer[n] == 2 ))
			then
				if [[ ${framespliter[n]} != "" ]]
				then
					echo ${framespliter[n]} $original_seq ${case_data_directory}/frametype.txt $original_seq2 2 "${seq_width[index]}" "${seq_height[index]}"
					${framespliter[n]} $original_seq ${case_data_directory}/frametype.txt $original_seq2 2 "${seq_width[index]}" "${seq_height[index]}"
					return_code=$?
					echo "split frames=$return_code"
					
					mv ${original_seq} ${original_seq}.org		
					original_backup=1						
					cp -f ${original_seq2} ${original_seq}				
					echo "./yuvconvert.exe $original_seq $original_seq_vqm ${seq_width[index]} ${seq_height[index]} 0"
					./yuvconvert.exe $original_seq $original_seq_vqm ${seq_width[index]} ${seq_height[index]} 0
					original_seq_size=`wc -c $original_seq_vqm | awk '{print $1}'`
				fi				
			fi
			
			#end adjustment
			
			#PSNR estimation			
			if (( enable_psnr_flag == 1 ))
			then
				psnr_file="${case_result_directory}/${seq_name[index]}_${tp_value[j]}_${n}_psnr.txt"				
				rm ${tmp_file} -f
				
				echo "${PSNR_EVALUATION} $original_seq $post_output_path ${psnr_file} ${tmp_file} ${seq_width[index]} ${seq_height[index]} "
				${PSNR_EVALUATION} $original_seq $post_output_path ${psnr_file} ${tmp_file} ${seq_width[index]} ${seq_height[index]}
								
				psnr_value=`cat ${tmp_file}`
				rm ${tmp_file} -f	
				echo "average psnr_value=${psnr_value}"
			fi
			
			#SSIM estimation			
			if (( enable_ssim_flag == 1 ))
			then
				ssim_file="${case_result_directory}/${seq_name[index]}_${tp_value[j]}_${n}_ssim.txt"				
				rm ${tmp_file} -f
				
				echo "${SSIM_EVALUATION} $original_seq $post_output_path ${ssim_file} ${tmp_file} ${seq_width[index]} ${seq_height[index]} "
				${SSIM_EVALUATION} $original_seq $post_output_path ${ssim_file} ${tmp_file} ${seq_width[index]} ${seq_height[index]}
								
				ssim_value=`cat ${tmp_file}`
				rm ${tmp_file} -f	
				echo "average ssim_value=${ssim_value}"
			fi
			
			#byte diff with reconstrcutive sequence
			if (( enable_recdiff_flag == 1 ))
			then
				recdiff_file="${case_result_directory}/${seq_name[index]}_${tp_value[j]}_${n}_recdiff.txt"				
				rm ${tmp_file} -f
				
				if [ -e $encoder_rec_output_path -a -e $post_output_path ]; then
			        echo "cmp $encoder_rec_output_path $post_output_path > $recdiff_file"
	                        cmp $encoder_rec_output_path $post_output_path > $recdiff_file
	                        else
	                        echo "error! either $encoder_rec_output_path or $post_output_path doesn't exist!"
	                        fi
				
			fi

#the end of chapter 7
	
#chapter 8 	the process of VQM analysis

			#VQM estimation
			if (( enable_vqm_flag == 1 ))
			then
				echo "ready to do vqm"

				#general the cntl file for vqm evaluation				
				cntl_content=`cat ./template.cntl`
			  	login_name=`whoami`

				processed_vqm="${case_data_directory}/${seq_name[index]}_${tp_value[j]}_${n}.uyvy"
				
				echo "./yuvconvert.exe $post_output_path $processed_vqm ${seq_width[index]} ${seq_height[index]} 0"
				./yuvconvert.exe $post_output_path $processed_vqm ${seq_width[index]} ${seq_height[index]} 0

				seq_size=`wc -c $processed_vqm | awk '{print $1}'`
				if (( original_seq_size != seq_size ))
				then
					echo "there must be a error! the processed file size is unequal with size of the original file size"
					exit -1 
				fi

				cntl_content=`echo "${cntl_content/_cur_user_/$login_name}"`
				cntl_content=`echo "${cntl_content/_original_yuv_path_/$original_seq_vqm}"`
				cntl_content=`echo "${cntl_content/_original_yuv_size_/$original_seq_size}"`

				cntl_content=`echo "${cntl_content/_processed_yuv_path_/$processed_vqm}"`
				cntl_content=`echo "${cntl_content/_processe_yuv_size_/$seq_size}"`

				cntl_content=`echo "${cntl_content//_frame_width_/${seq_width[index]}}"`
				cntl_content=`echo "${cntl_content//_frame_height_/${seq_height[index]}}"`
				cntl_content=`echo "${cntl_content/_processed_yuv_size_/$seq_size}"`
				cur_fps=${seq_fps[index]}
				if(( cur_fps <= 10 ))
				then
					cur_fps=10
				fi
				cntl_content=`echo "${cntl_content/_timing_format_/${TimingFormat[cur_fps]}}"`
				
				echo "Email Address: $login_name" > ${tmp_cntl_file}
				echo "Original File: $original_seq_vqm" >> ${tmp_cntl_file}
				echo "Original Bytes: $original_seq_size" >> ${tmp_cntl_file}
				echo "Number Processed: 1" >> ${tmp_cntl_file}
				echo "Processed File: $processed_vqm" >> ${tmp_cntl_file}
				echo "Processed Bytes: $seq_size" >> ${tmp_cntl_file}
				echo "Is Compressed: False" >> ${tmp_cntl_file}				 
				echo "Image Rows: ${seq_height[index]}" >> ${tmp_cntl_file}				 
				echo "Image Cols: ${seq_width[index]}" >> ${tmp_cntl_file}				
				echo "Timing Format: ${TimingFormat[cur_fps]}" >> ${tmp_cntl_file}				 
				echo "Scanning Pattern: Progressive" >> ${tmp_cntl_file}	 
				if ((vqm_paring_type == 0))
				then
					echo "Parsing Type: NONE" >> ${tmp_cntl_file}
				else
					echo "Parsing Type: SPLIT" >> ${tmp_cntl_file}
					
					#length of each vqm segment, should be 5-10s in time					
					let "vqm_segment_length=${cur_fps}*10"
					let "vqm_segment_freq=${cur_fps}*10"
					
					echo "Parsing Length: $vqm_segment_length" >> ${tmp_cntl_file}
					echo "Parsing Frequency: $vqm_segment_freq" >> ${tmp_cntl_file}
				fi
				echo "Spatial Calibration: VALUES" >> ${tmp_cntl_file}
				echo "Spatial Vertical: 0" >> ${tmp_cntl_file}
				echo "Spatial Horizontal: 0" >> ${tmp_cntl_file}
				echo "Valid Calibration: VALUES" >> ${tmp_cntl_file}
				echo "Valid Top: 2" >> ${tmp_cntl_file}
				echo "Valid Left: 4" >> ${tmp_cntl_file}
				echo "Valid Bottom: ${seq_height[index]}" >> ${tmp_cntl_file}
				echo "Valid Right: ${seq_width[index]}" >> ${tmp_cntl_file}
				echo "Gain Calibration: VALUES" >> ${tmp_cntl_file}
				echo "Luminance Gain: 1.0" >> ${tmp_cntl_file}
				echo "Luminance Offset: 0" >> ${tmp_cntl_file}
				echo "Temporal Calibration: VALUES" >> ${tmp_cntl_file}
				echo "Temporal Shift: 0" >> ${tmp_cntl_file}
				echo "Alignment Uncertainty: 0" >> ${tmp_cntl_file}
				echo "Calibration Frequency: 15" >> ${tmp_cntl_file}
				echo "Video Model: vconf_model" >> ${tmp_cntl_file}

				#call the vqm programm
				echo "${VQM_EVALUATION} ${tmp_cntl_file}"				
			        rm "/var/mail/$login_name" -f
			        rm "./vqm_tmp.sh" -f
				${VQM_EVALUATION} ${tmp_cntl_file}	
				if (($? != 0))
				then
					echo "error while calling vqm.exe, please check tmp.cntl"
					exit 1
				fi

				#checking the result emails are all arrived
				if (( vqm_paring_type == 0))
				then
					grep_string="model video-conference"
					grep_output_lines=14
				else
					vqm_segment=`echo "$frame_num" "$vqm_segment_length" "$vqm_segment_freq" | awk '{ printf( "%.0f\n", (1+(($1-$2)/$3)) ) }'`  #compute ($1/$2)
					echo "vqm_segment=$vqm_segment"
					grep_string="Parsing Summary"
					grep_output_lines=$vqm_segment
					let "grep_output_lines += 18"
				fi
				
				echo "grep_string=$grep_string"
				echo "grep_output_lines=$grep_output_lines"
				
				while ! [ -e "/var/mail/$login_name" ]
				do
					echo "/var/mail/$login_name does not exist! we will check it again in 1 seconds"
					sleep 1
				done
				sleep 2

				grep_output=`grep "$grep_string" /var/mail/$login_name -A $grep_output_lines`
				echo "grep_output=$grep_output"
				while [[ $grep_output == "" ]]
				do
					echo "the vqm result is not arrived yet! we will check it again in 1 seconds"
					grep_output=`grep "$grep_string" "/var/mail/$login_name" -A $grep_output_lines`
					sleep 1				
				done
				
				vqm_log="${case_result_directory}/${seq_name[index]}_${tp_value[j]}_${n}_vqm.log"
				echo "cp /var/mail/$login_name $vqm_log"						
				cp "/var/mail/$login_name" "$vqm_log"
				
				echo "$grep_output" > "./vqm_grep.txt"
				if (( vqm_paring_type == 0))
				then
					OLD_IFS=$IFS	
					IFS=":"
					while read -a line
					do
						if [[ ${line[0]} == "model video-conference" ]]
						then
							vqm_score=`echo ${line[1]}|tr -d " "`
							break;
						fi			
					done < "./vqm_grep.txt"				
					
					IFS=$OLD_IFS	
				else
					OLD_IFS=$IFS
					IFS="]"
					cur_seg=0
					total_score=0
					line_num=0
					while read -a line
					do
						let "line_num+=1"
						if (( line_num >= 6 && line_num < vqm_segment + 6 ))
						then
							total_score=`echo $total_score ${line[1]} | awk '{ printf( "%.4f\n", $1+$2 ) }'`  #compute ($1+$2)
							let "cur_seg+=1"
							echo $cur_seg $seg_vqm $total_score
						fi
					done < "./vqm_grep.txt"
					
					IFS=$OLD_IFS	

					vqm_score=`echo $total_score $cur_seg | awk '{ printf( "%.4f\n", $1/$2 ) }'`  #compute ($1+$2)
				fi					
				echo $vqm_score			

					
#					IFS="="
#					while read -a line
#					do
#						if [[ ${line[0]} == "Jerky or Unnatural Motion " ]]	
#						then
#							vqm_jerky=`echo ${line[1]}|tr -d " "`
#							echo "vqm_jerky=$vqm_jerky">>"./vqm_tmp.sh"
#						fi
#						
#						if [[ ${line[0]} == "Block Distortion " ]]
#						then	
#							vqm_distortion=`echo ${line[1]}|tr -d " "`
#							echo "vqm_distortion=$vqm_distortion">>"./vqm_tmp.sh"
#							break;
#						fi					
#					
#						if [[ ${line[0]} == "Blurring " ]]
#						then
#							vqm_blurring=`echo ${line[1]}|tr -d " "`
#							echo "vqm_blurring=$vqm_blurring">>"./vqm_tmp.sh"
#						fi
#					done < "./vqm_grep.txt"	
				
				vqm_file="${case_result_directory}/${seq_name[index]}_vqm.txt"


				echo "codec_name: ${codec_name[n]}, test points: ${tp_name} ${tp_value[j]}" >> $vqm_file					
				echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" >> $vqm_file
				if (( vqm_paring_type == 0))
				then
					start_line=1
				else
					start_line=4
				fi
				line_num=0
				while read line
				do
					let "line_num+=1"
					if (( line_num >= start_line ))
					then 
						echo "$line" >> $vqm_file
					fi
				done < "./vqm_grep.txt"
				echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++" >> $vqm_file
				echo "" >> $vqm_file
				echo "" >> $vqm_file
				echo "" >> $vqm_file

				rm "./vqm_grep.txt" -f

				vqm_value=$vqm_score
	
				#echo  "${tp_value[j]} $bitrate_value $psnr_value $vqm_value $ssim_value" >> ${avsnr_file[n]}
				
				rm $processed_vqm -f
			fi
			
			echo  "${tp_value[j]} $bitrate_value $psnr_value $vqm_value $ssim_value" >> ${avsnr_file[n]}
#end of chapter 8


#chapter 9 plot the curve of frame_PSNR frame_SSIM and frame_bit
				
		#Compute Bits of each frame
		if (( plot_frame_bits_flag == 1 ))
			then
				bits_file="${case_result_directory}/${seq_name[index]}_${tp_value[j]}_${n}_bits.txt"
				echo "./ComputeFrameBits.exe $extractor_output_path $bits_file"
				./ComputeFrameBits.exe $extractor_output_path $bits_file
			fi
			
		done
	
	
		if (( plot_frame_psnr_flag == 1 ))
		then
			plot_command=`cat ./template.gnu`
			png_out_path="$case_result_directory/${seq_name[index]}_${tp_value[j]}_psnr.png"
		        plot_name="${case_name} \n Frame-PSNR Plot \n ${seq_description[index]} ${tp_name}:${tp_value[j]}"
		        
			plot_command=`echo ${plot_command//_title_/$plot_name}`
			plot_command=`echo ${plot_command//_plot_path_/$png_out_path}` 
			plot_command=`echo ${plot_command//_xlabel_/"Frame"}` 
			plot_command=`echo ${plot_command//_ylabel_/"PSNR(db)"}` 
			plot_command=`echo ${plot_command//_keypos_/"top right"}` 
			
			for ((n=$n2; n <= $n3; n++))
			do
				psnr_file="${case_result_directory}/${seq_name[index]}_${tp_value[j]}_${n}_psnr.txt"	
				if (( n == n2 ))	
				then		
					plot_command=` echo "$plot_command plot '${psnr_file}' using 1:2  title '${codec_name[n]}' with lines linetype $n"`
				else
					plot_command=`echo "$plot_command '${psnr_file}' using 1:2  title '${codec_name[n]}' with lines linetype $n"`
				fi				
				if (( n < n3 ))
				then
					plot_command=`echo "${plot_command}, "`
				fi
			done
			echo $plot_command > ${tmp_gnu_file}
			gnuplot ${tmp_gnu_file}	
			if (($? != 0))
			then
				echo "error while calling gnuplot, please check tmp.gnu"
				exit 1
			fi
		fi
		
		if (( plot_frame_ssim_flag == 1 ))
		then
			plot_command=`cat ./template.gnu`
			png_out_path="$case_result_directory/${seq_name[index]}_${tp_value[j]}_ssim.png"
		        plot_name="${case_name} \n Frame-SSIM Plot \n ${seq_description[index]} ${tp_name}:${tp_value[j]}"
		        
			plot_command=`echo ${plot_command//_title_/$plot_name}`
			plot_command=`echo ${plot_command//_plot_path_/$png_out_path}` 
			plot_command=`echo ${plot_command//_xlabel_/"Frame"}` 
			plot_command=`echo ${plot_command//_ylabel_/"SSIM(%)"}` 
			plot_command=`echo ${plot_command//_keypos_/"top right"}` 
			
			for ((n=n2; n <= n3; n++))
			do
				ssim_file="${case_result_directory}/${seq_name[index]}_${tp_value[j]}_${n}_ssim.txt"	
				if (( n == n2 ))	
				then		
					plot_command=` echo "$plot_command plot '${ssim_file}' using 1:2  title '${codec_name[n]}' with lines linetype $n"`
				else
					plot_command=`echo "$plot_command '${ssim_file}' using 1:2  title '${codec_name[n]}' with lines linetype $n"`
				fi				
				if (( n < n3 ))
				then
					plot_command=`echo "${plot_command}, "`
				fi
			done
			echo $plot_command > ${tmp_gnu_file}
			gnuplot ${tmp_gnu_file}	
			if (($? != 0))
			then
				echo "error while calling gnuplot, please check tmp.gnu"
				exit 1
			fi
		fi

if (( plot_frame_perf_flag == 1 ))
		then
			plot_command=`cat ./template.gnu`
			png_out_path="$case_result_directory/${seq_name[index]}_${tp_value[j]}_perf.png"
		  plot_name="${case_name} \n Frame_CPU-load Plot \n ${seq_description[index]} ${tp_name}:${tp_value[j]}"
		        
			plot_command=`echo ${plot_command//_title_/$plot_name}`
			plot_command=`echo ${plot_command//_plot_path_/$png_out_path}` 
			plot_command=`echo ${plot_command//_xlabel_/"Frame"}` 
			plot_command=`echo ${plot_command//_ylabel_/"CPU time(ns)"}` 
			plot_command=`echo ${plot_command//_keypos_/"top right"}` 
			
			for ((n=n2; n <= n3; n++))
			do
				perf_file="${case_data_directory}/${seq_name[index]}_${tp_value[j]}_${n}_perf.txt"	
				if (( n == n2 ))	
				then		
					plot_command=` echo "$plot_command plot '${perf_file}' using 1:2  title '${codec_name[n]}' with lines linetype $n"`
				else
					plot_command=`echo "$plot_command '${perf_file}' using 1:2  title '${codec_name[n]}' with lines linetype $n"`
				fi				
				if (( n < n3 ))
				then
					plot_command=`echo "${plot_command}, "`
				fi
			done
			echo $plot_command > "./tmp.gnu"
			gnuplot "./tmp.gnu"	
			if (($? != 0))
			then
				echo "error while calling gnuplot, please check tmp.gnu"
				exit 1
			fi
		fi


		if (( plot_frame_bits_flag == 1 ))
		then
			plot_command=`cat ./template.gnu`
			png_out_path="$case_result_directory/${seq_name[index]}_${tp_value[j]}_bits.png"
		        plot_name="${case_name} \n \n Frame-FrameSize Plot \n ${seq_description[index]} \n ${tp_name}:${tp_value[j]}"
		        
			plot_command=`echo ${plot_command//_title_/$plot_name}`
			plot_command=`echo ${plot_command//_plot_path_/$png_out_path}` 
			plot_command=`echo ${plot_command//_xlabel_/"Frame"}` 
			plot_command=`echo ${plot_command//_ylabel_/"Bytes"}` 
			plot_command=`echo ${plot_command//_keypos_/"top right"}` 
			
			for ((n=n2; n <= n3; n++))
			do
				bits_file="${case_result_directory}/${seq_name[index]}_${tp_value[j]}_${n}_bits.txt"	
				if (( n == n2 ))	
				then		
					plot_command=` echo "$plot_command plot '${bits_file}' using 1:2  title '${codec_name[n]}' with lines linetype $n"`
				else
					plot_command=`echo "$plot_command '${bits_file}' using 1:2  title '${codec_name[n]}' with lines linetype $n"`
				fi				
				if (( n < n3 ))
				then
					plot_command=`echo "${plot_command}, "`
				fi
			done
			echo $plot_command > ${tmp_gnu_file}
			echo "plot frame_bits"
			gnuplot ${tmp_gnu_file}	
			if (($? != 0))
			then
				echo "error while calling gnuplot, please check tmp.gnu"
				exit 1
			fi		
		fi
	done
#the end of chapter 9

	
#chapter 10

	#write test report
	for (( n=n2; n <= n3; n++ ))
	do
		echo ${codec_name[n]} >> $test_data_path
		
		bitrate_tab=""
		psnr_tab=""
		vqm_tab=""
		ssim_tab=""
		
		if (( enable_bitrate_flag == 1 ))
		then
			bitrate_tab="KBitPS"
		fi
		
		if (( enable_psnr_flag == 1 ))
		then
			psnr_tab="PSNR"
		fi
		
		if (( enable_vqm_flag == 1 ))
		then
			vqm_tab="VQM"
		fi
		
		if (( enable_ssim_flag == 1 ))
		then
			ssim_tab="SSIM"
		fi
		
		echo "$tp_name $bitrate_tab $psnr_tab $vqm_tab $ssim_tab" >> $test_data_path

		cat ${avsnr_file[n]} >> $test_data_path	

		#compute difference of average PSNR/Bitrate
		if (( enable_bitrate_flag == 1 ))
		then
			if (( n > n2 ))
			then
				echo "./ComputeAvDiff.exe ${avsnr_file[n2]} ${avsnr_file[n]} $tp_num 4  $test_data_path"
				./ComputeAvDiff.exe ${avsnr_file[n2]} ${avsnr_file[n]} $tp_num 4 $test_data_path
			fi
		fi				
	done
	
	if (( tp_type == 2 ))  # type 2 for bitrate as testpoint
	then
		rc_data_path="$case_result_directory/rc_report.txt"
		if (( index == 1 ))
		then
			echo "case name: $case_name " > $rc_data_path	
		fi

		echo "testing sequence $index: ${seq_name[index]}" >> $rc_data_path

		
		for (( n = n2; n <= n3; n++ ))
		do
		
			echo ${codec_name[n]} >> $rc_data_path

			echo "Target_Bitrate Actual_Bitrate Error(%)" >> $rc_data_path 		

			total_diff=0						
			rate_error_file[n]="${case_result_directory}/${seq_name[index]}_${n}_re.txt"
			IFS=" "
			cat ${avsnr_file[n]} | while read -a line
			do
				rate_diff=`echo ${line[0]} ${line[1]} | awk "$AWKSCRIPT_DIFF"`
				rate_diff=`echo ${rate_diff}|tr -d "-"`
				 
				echo "${line[0]} ${line[1]} $rate_diff%" >> $rc_data_path
				echo "${line[0]} ${line[1]} $rate_diff" >> ${rate_error_file[n]}
				total_diff=`echo $total_diff $rate_diff} | awk "$AWKSCRIPT_ADD"`
				echo $total_diff > ${tmp_file}
			done
			total_diff=`cat ${tmp_file}`
			average_diff=`echo $total_diff $j | awk "$AWKSCRIPT_DIVIDE"`
			echo "$average_diff $total_diff $j"
			echo "average bitrate error over all test points is: ${average_diff}%" >> $rc_data_path
		done
		
		#plot the Bitrate- Rate error png
		plot_command=`cat ./template.gnu`
		png_out_path="$case_result_directory/${seq_name[index]}_rc.png"
	        plot_name="${case_name} \n Bitrate - Bitrate Error Plot \n ${seq_description[index]}"
	        
		plot_command=`echo ${plot_command//_title_/$plot_name}`
		plot_command=`echo ${plot_command//_plot_path_/$png_out_path}` 
		plot_command=`echo ${plot_command//_xlabel_/"kbps"}` 
		plot_command=`echo ${plot_command//_ylabel_/"error(%)"}` 
		plot_command=`echo ${plot_command//_keypos_/"top left"}` 
		
		echo $plot_command > ${tmp_gnu_file}		

		for ((n=n2; n <= n3; n++))
		do
			if (( n == n2 ))	
			then		
				plot_command=` echo "$plot_command plot '${rate_error_file[n]}' using "'($1/1000):($3)'" title '${codec_name[n]}' with linespoints pt $n"`
			else
				plot_command=`echo "$plot_command '${rate_error_file[n]}' using "'($1/1000):($3)'" title '${codec_name[n]}' with linespoints pt $n"`
			fi				
			if (( n < n3 ))
			then
				plot_command=`echo "${plot_command}, "`
			fi
		done
		
		echo $plot_command >> ${tmp_gnu_file}		
		gnuplot ${tmp_gnu_file}		
		if (($? != 0))
		then
			echo "error while calling gnuplot, please check tmp.gnu"
			exit 1
		fi	
			
		for ((n=n2; n <= n3; n++))
		do	
			rm ${rate_error_file[n]} -f
		done
	fi	
	
	#plot packet loss - rate distortion
	if (( enable_bitrate_flag == 0 )) #packet loss mode
	then
		x_data_pos=1
		y_data_pos=2
		if (( enable_psnr_flag == 1 ))
		then
			plot_command=`cat ./template.gnu`
			png_out_path="$case_result_directory/${seq_name[index]}_psnr_curv.png"
		        plot_name="${case_name} \n Packet Loss and PSNR \n ${seq_description[index]}"
		        
			plot_command=`echo ${plot_command//_title_/$plot_name}`
			plot_command=`echo ${plot_command//_plot_path_/$png_out_path}` 
			plot_command=`echo ${plot_command//_xlabel_/$tp_name}` 
			plot_command=`echo ${plot_command//_ylabel_/"PSNR(db)"}` 
			plot_command=`echo ${plot_command//_keypos_/"top right"}` 
			
			echo $plot_command > ${tmp_gnu_file}			
			echo "set ytics 0.5" >> ${tmp_gnu_file}
	
			plot_command=""	
			for ((n=n2; n <= n3; n++))
			do
				if (( n == n2 ))	
				then		
					plot_command=` echo "$plot_command plot '${avsnr_file[n]}' using $x_data_pos:$y_data_pos  title '${codec_name[n]}' with linespoints pt $n"`
				else
					plot_command=`echo "$plot_command '${avsnr_file[n]}' using $x_data_pos:$y_data_pos  title '${codec_name[n]}' with linespoints pt $n"`
				fi				
				if (( n < n3 ))
				then
					plot_command=`echo "${plot_command}, "`
				fi
			done
			echo $plot_command >> ${tmp_gnu_file}	
			gnuplot ${tmp_gnu_file}	
			if (($? != 0))
			then
				echo "error while calling gnuplot, please check tmp.gnu"
				exit 1
			fi			
			
			let "y_data_pos += 1"
		fi
		
		if (( enable_vqm_flag == 1 ))
		then
			plot_command=`cat ./template.gnu`
			png_out_path="$case_result_directory/${seq_name[index]}_vqm_curv.png"
		        plot_name="${case_name} \n Packet Loss and VQM \n ${seq_description[index]}"
		        
			plot_command=`echo ${plot_command//_title_/$plot_name}`
			plot_command=`echo ${plot_command//_plot_path_/$png_out_path}` 
			plot_command=`echo ${plot_command//_xlabel_/$tp_name}` 
			plot_command=`echo ${plot_command//_ylabel_/"VQM"}` 
			plot_command=`echo ${plot_command//_keypos_/"top right"}` 
			
			echo $plot_command > ${tmp_gnu_file}			
	
			plot_command=""	
			for ((n=n2; n <= n3; n++))
			do
				if (( n == n2 ))	
				then		
					plot_command=` echo "$plot_command plot '${avsnr_file[n]}' using $x_data_pos:$y_data_pos  title '${codec_name[n]}' with linespoints pt $n"`
				else
					plot_command=`echo "$plot_command '${avsnr_file[n]}' using $x_data_pos:$y_data_pos  title '${codec_name[n]}' with linespoints pt $n"`
				fi				
				if (( n < n3 ))
				then
					plot_command=`echo "${plot_command}, "`
				fi
			done
			echo $plot_command >> ${tmp_gnu_file}			
			gnuplot ${tmp_gnu_file} 	
			if (($? != 0))
			then
				echo "error while calling gnuplot, please check tmp.gnu"
				exit 1
			fi			
		fi			
		
		
	else 	
		#enable_bitrate_flag=1
		#plot rd-curve with gnuplot
		
		x_data_pos=2
		y_data_pos=3
		if (( enable_psnr_flag == 1 ))
		then
			plot_command=`cat ./template.gnu`
			png_out_path="$case_result_directory/${seq_name[index]}_rd_psnr.png"
		        plot_name="${case_name} \n Rate-Distortion(PSNR) \n  ${seq_description[index]}"
		        
			plot_command=`echo ${plot_command//_title_/$plot_name}`
			plot_command=`echo ${plot_command//_plot_path_/$png_out_path}` 
			
			plot_command=`echo ${plot_command//_ylabel_/"PSNR(db)"}` 
			plot_command=`echo ${plot_command//_keypos_/"top left"}` 
			
			plot_command=`echo ${plot_command//_xlabel_/"kbps"}` 

			echo $plot_command > ${tmp_gnu_file}
			plot_command=""					
			for ((n=n2; n <= n3; n++))
			do
				if (( n == n2 ))	
				then		
					plot_command=` echo "$plot_command plot '${avsnr_file[n]}' using $x_data_pos:$y_data_pos  title '${codec_name[n]}' with linespoints pt $n"`
				else
					plot_command=`echo "$plot_command '${avsnr_file[n]}' using $x_data_pos:$y_data_pos title '${codec_name[n]}' with linespoints pt $n"`
				fi				
				if (( n < n3 ))
				then
					plot_command=`echo "${plot_command}, "`
				fi
			done
			
			echo "set ytics 0.5;" >> ${tmp_gnu_file}	
			echo $plot_command >> ${tmp_gnu_file}			
			gnuplot ${tmp_gnu_file}	
			if (($? != 0))
			then
				echo "error while calling gnuplot, please check tmp.gnu"
				exit 1
			fi	
							
			let "y_data_pos += 1"			
		fi
			
		
		if (( enable_vqm_flag == 1 ))
		then
			plot_command=`cat ./template.gnu`
			png_out_path="$case_result_directory/${seq_name[index]}_rd_vqm.png"
		        plot_name="${case_name} \n Rate-Distortion(VQM) \n  ${seq_description[index]}"
		        
			plot_command=`echo ${plot_command//_title_/$plot_name}`
			plot_command=`echo ${plot_command//_plot_path_/$png_out_path}` 
			
			plot_command=`echo ${plot_command//_ylabel_/VQM}` 
			plot_command=`echo ${plot_command//_keypos_/top left}` 
			
			plot_command=`echo ${plot_command//_xlabel_/kbps}` 

			echo $plot_command > ${tmp_gnu_file}			
			plot_command=""					
			for ((n=n2; n <= n3; n++))
			do
				if (( n == n2 ))	
				then		
					plot_command=` echo "$plot_command plot '${avsnr_file[n]}' using $x_data_pos:$y_data_pos  title '${codec_name[n]}' with linespoints pt $n"`
				else
					plot_command=`echo "$plot_command '${avsnr_file[n]}' using $x_data_pos:$y_data_pos title '${codec_name[n]}' with linespoints pt $n"`
				fi				
				if (( n < n3 ))
				then
					plot_command=`echo "${plot_command}, "`
				fi
			done
			
			echo $plot_command >> ${tmp_gnu_file}		
			gnuplot ${tmp_gnu_file}	
			if (($? != 0))
			then
				echo "error while calling gnuplot, please check tmp.gnu"
				exit 1
			fi			
		fi

		x_data_pos=2
		y_data_pos=5
		if (( enable_ssim_flag == 1 ))
		then
			plot_command=`cat ./template.gnu`
			png_out_path="$case_result_directory/${seq_name[index]}_rd_ssim.png"
		        plot_name="${case_name} \n Rate-Distortion(SSIM) \n  ${seq_description[index]}"
		        
			plot_command=`echo ${plot_command//_title_/$plot_name}`
			plot_command=`echo ${plot_command//_plot_path_/$png_out_path}` 
			
			plot_command=`echo ${plot_command//_ylabel_/"SSIM(%)"}` 
			plot_command=`echo ${plot_command//_keypos_/"top left"}` 
			
			plot_command=`echo ${plot_command//_xlabel_/"kbps"}` 

			echo $plot_command > ${tmp_gnu_file}		
			plot_command=""					
			for ((n=n2; n <= n3; n++))
			do
				if (( n == n2 ))	
				then		
					plot_command=` echo "$plot_command plot '${avsnr_file[n]}' using $x_data_pos:$y_data_pos  title '${codec_name[n]}' with linespoints pt $n"`
				else
					plot_command=`echo "$plot_command '${avsnr_file[n]}' using $x_data_pos:$y_data_pos title '${codec_name[n]}' with linespoints pt $n"`
				fi				
				if (( n < n3 ))
				then
					plot_command=`echo "${plot_command}, "`
				fi
			done
			
			#echo "set ytics 0.5;" >> ${tmp_gnu_file}
			echo $plot_command >> ${tmp_gnu_file}		
			gnuplot ${tmp_gnu_file} 	
			if (($? != 0))
			then
				echo "error while calling gnuplot, please check tmp.gnu"
				exit 1
			fi	
							
		fi
		
		#for combine test purpose	
			if(( case_combine == 1 ))
			then
				for ((n=n2; n <= n3; n++))
				do
					psnr_file=$1
					cp -f ${avsnr_file[n]} ${psnr_file}.rd.${n}
				done
			fi
			
	fi
done 

rm -f ${tmp_gnu_file} 
rm -f ${tmp_cntl_file}
rm -f "${case_result_directory}/*.uyvy"

echo  "delete_all_yuv_flag = $delete_all_yuv_flag"
if (( ${delete_all_yuv_flag} == 1 ))
then
        rm -f ${case_result_directory}/*.yuv
        echo "all the YUV files in result folder ${case_result_directory} are removed"
fi


echo  "delete_all_data_flag = $delete_all_data_flag"
if (( ${delete_all_data_flag} == 1 ))
then
        #if [[ ${test_data_directory} != ${test_result_directory} ]]
        #then
                rm -f ${case_data_directory}/*.yuv
								rm -f ${case_data_directory}/*.uyvy
        echo "all the YUV files in data folder ${case_data_directory} are removed"
        #fi
fi

echo "this is the end of case execution."

exit 0
