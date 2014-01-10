#!/bin/sh


SGE_enable=1
removeYUV=1
case_name="JM_encoder"
case_data_directory="./"
case_result_directory="./"

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

chmod 644 $1

echo the JM encoder compression process starts
date

#------------------------prepare the directory

time=$(date +%m-%d-%y)_$(date +%k-%M-%S)
time2=`echo ${time}|tr " " "0"`

echo $case_name
case_name=`echo ${case_name//" "/"_"}`
echo $case_name

case_result_total="${test_result_directory}${case_name}_result_total"
case_result_directory="${test_result_directory}${case_name}_${time2}"

if ! [ -d ${case_result_total} ]
then
mkdir ${case_result_total}
fi
echo "the results are listed at ${case_result_total}"
chmod 777 ${case_result_total}

# in SGE environment, if the data folder exists, will use another different folder combined with a index value.
if(( SGE_enable == 1 ))
	then
	
	for ((index=1; index <= 200; index++))
	do
			case_result_directory_new="${case_result_directory}_${index}"
			if ! [ -d ${case_result_directory_new} ]
			then 
			case_result_directory=${case_result_directory_new}
			break
			fi
	done 
	
	if(( index == 200 ))
	then
	echo "concurrent tasks has reached the Maximinum value in SGE, stop the tasks!"
	exit -2 
	fi
fi

mkdir $case_result_directory

echo "test result will be stored in: $case_result_directory"


if ! [ -d ${case_result_directory} ]
then
	echo "can not create directory ${case_result_directory}"
	exit -1
fi


#-------------------------JM encoder process

cp -f ./lencod.exe  ${case_result_directory}
cp -f ./$1  ${case_result_directory}
cd ${case_result_directory}

./lencod.exe -d $1

cd ..

cp -f ${case_result_directory}/*.264  ${case_result_total}

if(( removeYUV == 1 ))
then
rm -f ${case_result_directory}/*.yuv
fi

echo the JM encoder compression process ends
date
exit 0
