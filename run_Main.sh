#!/bin/bash
#usage  runGetCodecRepositoryAddr  $configurfile
runGetCodecRepositoryAddr()   
{
	if [ ! $# -eq 1  ]
	then 
		echo "no parameter!"
		return 1
	fi
	
	local Configurefile=$1
	local RepositotyAddr=""
	
	while read line
	do
		if [[ $line =~ ^RepositoryAddr  ]]
		then
			RepositotyAddr=`echo $line | awk '{print $2}'`
			break
		fi
	done <${Configurefile}
	
	echo "${RepositotyAddr}"
}
#usage  runCheckoutAllTestPassStatus  $CurrDir
#eg  output log for one YUV looks like:
#../AllTestData/TestSetCIF/result/  TestSetCIF_Logi7500_320x192.yuv_AllCaseOutput.csv
#../AllTestData/TestSetCIF/result/  ../result/TestSetCIF_Logi7500_320x192.yuv.TestLog
#../AllTestData/TestSetCIF/result/  ../result/TestSetCIF_Logi7500_320x192.yuv.Summary
#../AllTestData/TestSetCIF/result/   ../result/TestSetCIF_Logi7500_320x192.yuv.passFlag
runCheckoutAllTestPassStatus()
{
	if [ ! $# -eq 1  ]
	then 
		echo "no parameter!"
		return 1
	fi
	local CurrDir=$1
	local ResultDir="${CurrDir}/result"
	local IssueData="${CurrDir}/result/issue"
	local TestLog="${CurrDir}/result/testlog"
	
	local TestDataSpace="${CurrDir}/AllTestData"
	local  OverAllReportFile="${CurrDir}/result/AllTestReport.csv"
	
	
	#clean all hostory data
	if [[  ${ResultDir} =~ /opt/  ]]
	then
		rm -rf  ${ResultDir}
	fi
	
	mkdir ${ResultDir}
	mkdir ${IssueData}
	mkdir ${TestLog}
	
	
	#copyfile
	for Set in ${TestDataSpace}/*
	do
		cd $Set
		for LogFile in "./result/*"
		do
			cp  ${LogFile}  ${TestLog}
		done
		
		for IssueFile in "./issue/*"
		do
			cp  ${IssueFile}  ${IssueData}
		done
				
		for SummaryFile in "./result/*.Summary"
		do
			cat   ${SummaryFile} >>${OverAllReportFile}
		done
			
	done
	
	cd ${CurrDir}
	#check whether all pass or not
	let "issueNum=0"
	for file in ${IssueData}/*
	do
	    let "issueNum++"
	done
	
	if [  ${issueNum} -gt 0 ]
	then
		echo "AllPass"
	else
		echo "Unpass"
	fi
	
}
#usage  runMain  $Configurefile $CurrDir
runMain()
{
	echo $#
	if [ ! $# -eq 2  ]
	then 
		echo "no parameter!"
		return 1
	fi
	
	
	local Configurefile=$1
	local CurrDir=$2
	local RepositoryAddr=""
	
	echo ""
	echo "***********************************************"
	echo "all test start!"
	
	echo ""
	echo "***********************************************"
	echo "prepare all test data folder"
	./run_PrepareTestSpace.sh  ${Configurefile}
	echo ""
	echo "***********************************************"
	echo "running all test, please wait......"
	echo ""
	./run_TestAllSequence.sh   ${Configurefile} 
	
	echo ""
	echo "final checking ...."
	Flag=`runCheckoutAllTestPassStatus  ${CurrDir}`
	
	echo $Flag
	if [[ "$Flag" =~ "AllPass"  ]]
	then
		echo "all case pass!"
		exit 0
	else
		echo "some cases failed!"
		exit 1
	fi
	
}
cfgfile=$1
CurrDir=`pwd`
echo "CurrDir is ${CurrDir}"
runMain ${cfgfile}  ${CurrDir}

