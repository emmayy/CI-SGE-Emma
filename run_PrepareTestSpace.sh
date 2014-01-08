#!/bin/bash


#global variable
ConfigureFile=$1
WorkingDirectory=`pwd`

TestFolder=${WorkingDirectory}/AllTestData
ExePath=${WorkingDirectory}/exe
CfgPath=${WorkingDirectory}/cfg
BatPath=${WorkingDirectory}/bats

declare -a TestSetIndex
declare -a TestSetName
TestSetIndex=(TestSetCIF  TestSetVGA TestSetXGA TestSet720p TestSet1080p  TestSetHEVC TestSetSpecial)
TestSetName=(CIF VGA XGA 720p 1080p HEVC Special)


#usage runSafeDelete $FileDir
runSafeDelete()
{
	#parameter check! 
	if [ ! $# -eq 1  ]
	then
		echo "need parameters for this function!"
		return 1
	 fi
	 
	#non-Test folder check
	local FileDir=$1
	if [[ !  "$FileDir" =~ ^/opt/  ]]
	then
		echo "try to delete non-SGE file! file:$FileDir"
		return 1
	fi
	#for other SGE data protection
	#eg /opt/VideoTest/eleiterm depth=4
	local FileDirDepth=`echo ${FileDir} | awk 'BEGIN {FS="/"} {print NF}'`
	if [  $FileDirDepth -lt 3 ]
	then
		echo "FileDepth is  $FileDirDepth not matched the minimand depth(5)"
		echo "unsafe delete! try to other SGE data file: $FileDir"
		return 1
	fi
	
	#delete if all check pass
	echo "delete iterm is:  $FileDir"
	rm -rf ${FileDir}
}

#get configure info 
#usage  runParseConfigure $ConfigureFile
runParseConfigure()
{
	#parameter check! 
	if [ ! $# -eq 1  ]
	then
		echo "need parameters for this function!"
		return 1
	fi

	local ConfigureFile=$1

	#read configure file and get the list of test sequences
	while read line
	do
		TestSetName=`echo $line | awk 'BEGIN {FS="[:\r]"} {print $1}'`

		if [[ ${TestSetName} =~ ^TestSetCIF  ]]
		then
			TestSetCIF=`echo $line | awk 'BEGIN {FS="[:\r]"} {print $2}'`
			
		elif [[ ${TestSetName} =~ ^TestSetVGA  ]]
		then
			TestSetVGA=`echo $line | awk 'BEGIN {FS="[:\r]"} {print $2}'`

		elif [[ ${TestSetName} =~ ^TestSetXGA  ]]
		then
			TestSetXGA=`echo $line | awk 'BEGIN {FS="[:\r]"} {print $2}'`

		elif [[ ${TestSetName} =~ ^TestSet720p  ]]
		then
			TestSet720p=`echo $line | awk 'BEGIN {FS="[:\r]"} {print $2}'`

		elif [[ ${TestSetName} =~ ^TestSet1080p  ]]
		then
			TestSet1080p=`echo $line | awk 'BEGIN {FS="[:\r]"} {print $2}'`

		elif [[  ${TestSetName} =~ ^TestSetHEVC   ]]
		then
			TestSetHEVC=`echo $line | awk 'BEGIN {FS="[:\r]"} {print $2}'`
			
		elif [[  ${TestSetName} =~ ^TestSetSpecial   ]]
		then
			TestSetSpecial=`echo $line | awk 'BEGIN {FS="[:\r]"} {print $2}'`
		fi

	done <${ConfigureFile}

}

##create all test set folder
#usage runSequenceFolderPrepare $TestSet  $CaseSetName  $SequenceSet
runSequenceFolderPrepare()
{
	NumParameter=$#
	if [ $NumParameter -gt 2  ]
	then
		declare -a SequenceSet
		SequenceSet=(`echo $@`)
		local SetName=$1
		local CaseSetName=$2

		local SetIssueFolder=${TestFolder}/${SetName}/issue
		local SetResultFolder=${TestFolder}/${SetName}/result

		mkdir -p ${SetIssueFolder}
		mkdir -p ${SetResultFolder}
		for((i=2;i<$NumParameter; i++))
		do
			local Sequence=${SequenceSet[$i]}
	        local SubFolder=${TestFolder}/${SetName}/${Sequence}
			echo "subfolder is $SubFolder"

	        mkdir -p ${SubFolder}
			cd ${SubFolder}
    	    
			cp -rf  ${ExePath}/*  ${SubFolder}
        	cp -rf  ${BatPath}/*  ${SubFolder}
        	cp -rf  ${CfgPath}/*  ${SubFolder}

			#generate .sge file
		 	#${SubFolder}/run_GenerateSGEFile.sh
			#generate case.csv file
			
			local CaseFileName=${SetName}_${Sequence}.csv
			echo "case parameters are :  case.cfg  ${CaseFileName}  ${CaseSetName}"
			${SubFolder}/run_GenerateCase.sh  case.cfg  ${CaseFileName}  ${CaseSetName} 
			chmod 777 ./*

		done
	fi

}


#usage
runMain()
{
	
	#clear all previous test data 
	if [ -d ${TestFolder} ]
	then
		runSafeDelete ${TestFolder}
	fi

	#parse configure info
	runParseConfigure  ${ConfigureFile}
	
	#create all test set folder
	let "setindex=0"
	for TestSet in ${TestSetIndex[@]}
	do
		eval SequenceSet=\$$TestSet
		echo "test Set is $SequenceSet"
		
		if [ $setindex -eq 0  ]
		then
			 CaseSetName="CIF"
		else
			 CaseSetName=${TestSetName[$setindex]}
		fi
		echo "case set name is $CaseSetName"
		echo ""
		runSequenceFolderPrepare $TestSet  $CaseSetName  $SequenceSet
	   let "setindex+=1"
	done

}

#call main function
runMain



