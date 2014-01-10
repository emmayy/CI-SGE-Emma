#!/bin/bash

#

#usage  runGlobalVariableDef
runGlobalVariableDef()
{
	WorkingDirectory=`pwd`
	TestFolder=${WorkingDirectory}/AllTestData
	
	declare -a TestSetIndex	
}

#usage  runGlobalVariableInital  $ConfigureFile
runGlobalVariableInital() 
{
	WorkingDirectory=`pwd`
	TestFolder=${WorkingDirectory}/AllTestData
	
	TestSetIndex=(TestSetCIF  TestSetVGA  TestSetXGA  TestSet720p TestSet1080p TestSetHEVC TestSetSpecial)
}


#usage runGetTestSetInfo  $Configurefile
#read configure file and get the list of test sequences
runGetTestSetInfo()
{
	
	local CaseFile="./cfg/case.cfg"


	echo ""
	echo "parse test set info"
	echo "cofigurefile is ${CaseFile}"
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
		elif [[ ${TestSetName} =~ ^TestSetHEVC  ]]
		then
			TestSetHEVC=`echo $line | awk 'BEGIN {FS="[:\r]"} {print $2}'`
		elif [[ ${TestSetName} =~ ^TestSetSpecial  ]]
		then
			TestSetSpecial=`echo $line | awk 'BEGIN {FS="[:\r]"} {print $2}'`
		fi

	done <${CaseFile}

}



#test all sequence under one  test set
#usage  SubmitOneSequence  $SetName $Sequence_01  $Sequence_02 ....
runTestOneSet()
{
	if [ ! $# -gt 1  ]
	then 
		echo "Test one set: $1 there is no yuv in this set!"
		return 1
	fi

	local NumParameter=$#
	local SetName=$1
	local SequenceName=""
	local SequenceDir=""
	
	declare -a SequenceSet
	SequenceSet=(`echo $@`)

	for((i=1;i<$NumParameter; i++))
	do
		SequenceName=${SequenceSet[$i]}
		SetName=$1
		SequenceDir=${TestFolder}/${SetName}/${SequenceName}
		
		echo " "
		echo "***************************************************************"
		echo "$SequenceName is under testing...."
		echo "SequenceDir is $SequenceDir"

		cd ${SequenceDir}
		./run_OneSequenceAllCase.sh
		
		#go back to working dir
		cd ${WorkingDirectory}
	done

}

runTestAllSequence()
{

	TestSetIndex=(TestSetCIF  TestSetVGA  TestSetXGA  TestSet720p TestSet1080p TestSetHEVC TestSetSpecial)
	for TestSet in ${TestSetIndex[@]}
	do
		eval TestSequenceSet=\$$TestSet
		echo ""
		echo  "***************************************************************"
		echo "test Set is $TestSequenceSet"
		echo ""
		runTestOneSet  $TestSet  $TestSequenceSet
	done

}

#usage  runMain $Configurefile
runMain()
{
	if [ ! $# -eq 1  ]
	then 
		echo "no parameter!"
		return 1
	fi
	
	local Configurefile=$1
	echo "main"
	echo "main cfg $Configurefile"
	
	runGlobalVariableDef
	runGlobalVariableInital
	echo "main cfg $Configurefile"
	runGetTestSetInfo    ${ConfigureFile}
	echo "main cfg $Configurefile"
	runTestAllSequence
}


#run main function
CfgFile=$1
echo "configure file is $CfgFile "
runMain  ${CfgFile}




