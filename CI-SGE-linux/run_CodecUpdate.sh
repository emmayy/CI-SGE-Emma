#!/bin/bash



#usage runSafeDelete $FileDir
runSafeDelete()
{
	#parameter check! 
	if [ ! $# -eq 1  ]
	then
		echo "need parameters for this function!"
		return 1
	 fi
	#non-SGE folder check
	local FileDir=$1
	if [[ !  "$FileDir" =~ ^/opt/   ]]
	then
		echo "try to delete non-SGE file! file:$FileDir"
		return 1
	fi
	#for other SGE data protection
	#eg /opt/CI-XX/iterm  depth=4
	local FileDirDepth=`echo ${FileDir} | awk 'BEGIN {FS="/"} {print NF}'`
	if [  $FileDirDepth -lt 3 ]
	then
		echo "FileDepth is  $FileDirDepth not matched the minimand depth(5)"
		echo "unsafe delete! try to other SGE data file: $FileDir"
		return 1
	fi

	#delete if all check pass
	echo "delete iterm is:  $FileDir"	
	if [ -d ${FileDir} ]
	then
		rm -rf ${FileDir}
	else
		rm -f  ${FileDir}
	fi	

}



#usage runCodecUpdate  $GitAddr   $SourceDir
runCodecUpdate()
{
	if [ ! $# -eq 2  ]
	then
		echo "need parameters for this function!"
		return 1
	fi
	
	local GitAddr=$1
	local SourceCodeDir=$2	
	local CurrDir=`pwd`
	
	if [ ! -d ${SourceCodeDir}  ]
	then
		mkdir -p  ${SourceCodeDir}
		git clone  ${GitAddr}  ${SourceCodeDir}
	else
		cd ${SourceCodeDir}
		git pull
		cd ${CurrDir}
	fi	
	
}

#usage runBuild  $SourceCodeFolder
runBuild()
{
	if [ ! $# -eq 1  ]
	then
		echo "need parameters for this function!"
		return 1
	fi
	
	local CodecFolder=$1

	#use master branch as test branch
	local CurrDir=`pwd`
	cd ${CodecFolder}
	git checkout master
	sudo make clean
	sudo make

	cd  ${CurrDir}	
	
	
	return 0
}


#copy the build result from codec repository to Test Space
#usage runCopyCodec $CodecFolder $ExeFolder
runCopyCodec()
{
	if [ ! $# -eq 2  ]
	then
		echo "need parameters for this function!"
		return 1
	fi
	
	local CodecFolder=$1
	local exeFolder=$2
	
	local CodecBinDir="codec/build/linux/bin"
	local CodecVPBinDir="processing/bin"
	local CodecCfgDir="testbin"
	
	#copy cfg file
	cp  -f ${CodecFolder}/${CodecCfgDir}/*.cfg   ${exeFolder}
	cp  -f ${CodecFolder}/*.a   ${exeFolder}
	cp  -f ${CodecFolder}/h264*   ${exeFolder}

}


#multi codec test uses the same cfg file
#usge  runUnifyCodecCfgFile  $TargetCodecDir  $BenchmarkCodecDir
runUnifyCodecCfgFile()
{
	if [ ! $# -eq 2  ]
	then
		echo "need parameters for this function!"
		return 1
	fi
			
	local TargetCodecDir=$1  
	local BenchmarkCodecDir=$2
	
	#delete those not the latest version cfg file
	for file in ${BenchmarkCodecDir}/*.cfg
	do
		runSafeDelete $file
	done
	
	#copy
	cp -f ${TargetCodecDir}/*.cfg   ${BenchmarkCodecDir}
	
}


#checkout and build all codec 
#usage  runMain $ReposAddr   $SourceCodeFolder $exeFolder
runMain()
{
	if [ ! $# -eq 3  ]
	then
		echo "need parameters for this function!"
		return 1
	fi
	local ReposAddr=$1
	local SourceFolder=$2
	local ExeFolder=$3
	local BenchmarkCodecDir="${ExeFolder}/codec_benchmark"
	local TargetCodecDir="${ExeFolder}/codec_target"
	
	if [ -e ${logfile}  ]
	then 
		rm -f ${logfile}
	fi
	
	echo "repository is ${ReposAddr}"
	echo "source code folder is ${SourceFolder}"
	
	#checkout all codec
	echo ""
	echo "codec updating from repository......"
	runCodecUpdate   ${ReposAddr} ${SourceFolder}
	echo ""
	echo "all codec have been checked out!" 
	
	#build and copy build result to test space 
	echo ""
	echo "building codec......"
    runBuild  ${SourceFolder}
	echo "all codec have been built!"
	echo ""
	echo "copy build result to test space"
	runCopyCodec ${SourceFolder}  ${TargetCodecDir}
	
	# unify the test cfg files
	echo ""
	echo " unifying  test cfg files...... "
	runUnifyCodecCfgFile  ${TargetCodecDir}  ${BenchmarkCodecDir}
	
}

#main*************
#important note: all dir format should be  used as  full path

CurrDir=`pwd`
GitAddr=$1  #"https://github.com/cisco/openh264"
echo $1
echo "repository address is $GitAddr"

SourDir="../Source-openh264-git"
ExeDir="${CurrDir}/exe"

echo "using main to check out code and build automatically.."
runMain  "${GitAddr}"   ${SourDir}  ${ExeDir}


