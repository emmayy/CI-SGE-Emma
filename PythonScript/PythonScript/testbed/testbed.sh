#!/bin/sh


# 1------define the sequences and decoders

SGE_enable=1
case_name="Comparision wels test"
case_data_directory="./"
case_result_directory="./"

removeYUV=0
stream_number=50
steamroot="./bitstream"
stream[1]="foreman_cif_300_qp20"
stream[2]="foreman_cif_300_qp22"
stream[3]="foreman_cif_300_qp24"
stream[4]="foreman_cif_300_qp26"
stream[5]="foreman_cif_300_qp28"
stream[6]="foreman_cif_300_qp30"
stream[7]="foreman_cif_300_qp32"
stream[8]="foreman_cif_300_qp34"
stream[9]="foreman_cif_300_qp36"
stream[10]="foreman_cif_300_qp40"
stream[11]="horse-riding-VGA-800_qp20"
stream[12]="horse-riding-VGA-800_qp22"
stream[13]="horse-riding-VGA-800_qp24"
stream[14]="horse-riding-VGA-800_qp26"
stream[15]="horse-riding-VGA-800_qp28"
stream[16]="horse-riding-VGA-800_qp30"
stream[17]="horse-riding-VGA-800_qp32"
stream[18]="horse-riding-VGA-800_qp34"
stream[19]="horse-riding-VGA-800_qp36"
stream[20]="horse-riding-VGA-800_qp40"
stream[21]="mobile_300_qp20"
stream[22]="mobile_300_qp22"
stream[23]="mobile_300_qp24"
stream[24]="mobile_300_qp26"
stream[25]="mobile_300_qp28"
stream[26]="mobile_300_qp30"
stream[27]="mobile_300_qp32"
stream[28]="mobile_300_qp34"
stream[29]="mobile_300_qp36"
stream[30]="mobile_300_qp40"
stream[31]="movie_show_VGA_2790_qp20"
stream[32]="movie_show_VGA_2790_qp22"
stream[33]="movie_show_VGA_2790_qp24"
stream[34]="movie_show_VGA_2790_qp26"
stream[35]="movie_show_VGA_2790_qp28"
stream[36]="movie_show_VGA_2790_qp30"
stream[37]="movie_show_VGA_2790_qp32"
stream[38]="movie_show_VGA_2790_qp34"
stream[39]="movie_show_VGA_2790_qp36"
stream[40]="movie_show_VGA_2790_qp40"
stream[41]="silent_300_qp20"
stream[42]="silent_300_qp22"
stream[43]="silent_300_qp24"
stream[44]="silent_300_qp26"
stream[45]="silent_300_qp28"
stream[46]="silent_300_qp30"
stream[47]="silent_300_qp32"
stream[48]="silent_300_qp34"
stream[49]="silent_300_qp36"
stream[50]="silent_300_qp40"



decodec_number=2
decoder[1]="ldecod.exe"
decoder[2]="h264dec.exe"
decoder[3]="ldecod.exe"

outputyuv[1]=""
outputyuv[2]=""



# 2-------define the input/output directories

echo "the execution of the case is started!"
echo "comparision test between JM deocder and Wels decoder."
tb_version=2.0
echo "the version of standalone testbed is ${tb_version}"

date

time=$(date +%m-%d-%y)_$(date +%k-%M-%S)
time2=`echo ${time}|tr " " "0"`

echo $case_name
case_name=`echo ${case_name//" "/"_"}`
echo $case_name

case_data_directory="${test_data_directory}${case_name}_${time2}"

# in SGE environment, if the data folder exists, will use another different folder combined with a index value.
if(( SGE_enable == 1 ))
	then
	
	for ((index=1; index <= 200; index++))
	do
			case_data_directory_new="${case_data_directory}_${index}"
			if ! [ -d ${case_data_directory_new} ]
			then 
			case_data_directory=${case_data_directory_new}
			break
			fi
	done 
	
	if(( index == 200 ))
	then
	echo "concurrent tasks has reached the Maximinum value in SGE, stop the tasks!"
	exit -2 
	fi
fi

mkdir $case_data_directory
chmod 777 $case_data_directory

case_data_directory="${case_data_directory}/data"

echo "test data will be stored in: $case_data_directory"
mkdir $case_data_directory



if ! [ -d ${case_data_directory} ]
then
	echo "can not create directory $case_name"
	exit -1
fi

chmod 777 $case_data_directory

case_result_directory="${test_result_directory}${case_name}_${time2}"

if(( SGE_enable == 1 ))
then
case_result_directory="${case_result_directory}_${index}"
fi

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
chmod 777 $case_result_directory


# 3------------the test procedure

for ((index=1; index <= stream_number ; index++))
do
		if ! [ -e  ${steamroot}/${stream[index]}.264 ]
		then
		echo "the stream file ${stream[index]}.264 does not exist"
		# exit -1
		continue
		fi
		
		cp -f ${steamroot}/${stream[index]}.264 ${case_data_directory}/${stream[index]}.264
		
done

rm -f "${case_data_directory}/test.264"

for ((index=1; index <= stream_number ; index++))
do
 		if ! [ -e  ${case_data_directory}/${stream[index]}.264 ]
		then
		echo "the stream file ${case_data_directory}/${stream[index]}.264 does not exist"
		# exit -1
		continue
		fi
		
		echo "testing stream is ${case_data_directory}/${stream[index]}.264"		
		cp -f ${case_data_directory}/${stream[index]}.264 "${case_data_directory}/test.264"
		
		if ! [ -e  "${case_data_directory}/test.264" ]
		then
		echo "the stream file (test.264) does not exist"
		exit -1
		fi
		
    for ((cn=1; cn <= decodec_number ; cn++))
    do
    
	   	if ! [ -e  ${decoder[cn]} ]
			then
			echo "the decoder does not exist"
			exit -1
			fi
			echo "the tested decoder is ${decoder[cn]}"
			
			if(( cn == 2 ))
			then
			echo ./${decoder[cn]} "${case_data_directory}/test.264" "${case_data_directory}/test.yuv"
			./${decoder[cn]} "${case_data_directory}/test.264" "${case_data_directory}/test.yuv"
			
			else
			echo ./${decoder[cn]} -i "${case_data_directory}/test.264" -o "${case_data_directory}/test.yuv"
			./${decoder[cn]} -i "${case_data_directory}/test.264" -o "${case_data_directory}/test.yuv"
			fi
			
			outputyuv[cn]=${stream[index]}_${cn}.yuv
			mv "${case_data_directory}/test.yuv" ${case_data_directory}/${outputyuv[cn]}
			chmod 777 ${case_data_directory}/${outputyuv[cn]}
		   
    done
    
    outputdiff=${stream[index]}.diff
    echo "cmp -b ${case_data_directory}/${outputyuv[1]} ${case_data_directory}/${outputyuv[2]} > ${case_result_directory}/$outputdiff"
    cmp ${case_data_directory}/${outputyuv[1]} ${case_data_directory}/${outputyuv[2]} > ${case_result_directory}/$outputdiff
    
#    if (($? != 0))
#			then
#					echo "error while calling cmp.exe, please check again!"
#					exit 1
#			fi
done

# 4----------clear the YUV files and reserve the diff files

if(( removeYUV== 1 ))
then
rm -f ${case_data_directory}/*.yuv
fi

rm -f "${case_data_directory}/test.264"

echo the decoder compare test ends
date
exit 0

