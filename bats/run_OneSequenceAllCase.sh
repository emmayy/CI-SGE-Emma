#!/bin/bash

#***********************************************************
#Test all cases for one sequence to check that the target codec is the same as benchmark codec
#output info lovated in ../AllTestData/${TestSetIndex}/result/
#         pass case number, unpass case number total case number
#         ${TestSetIndex}_${TestSequenceName}_AllCaseOutput.csv
#         ${AllCaseConsoleLogFile}
#         ${CaseSummaryFile}
#         ${FlagFile}
#***********************************************************

#***********************************************************

#global variable definition
#usage runGlobalVariableDef
runGlobalVariableDef()
{
	CurrentDir=""
	TestSetDir=""
	WorkDir=""
	
	#test data space
	FinalResultPath=""
	IssueDataPath=""
	TempDataPath=""
	
	#for test sequence info
	TestSequencePath=""
	TestSetIndex=""
	TestSequenceName=""
	SequencePath=""
	PicW=""
	PicH=""

	#test cfg file and test info output file
	ConfigureFile=""
	AllCaseFile=""
	#xxx.csv
	AllCasePassStatusFile=""
	
	#initial codec folder 
	declare  -a aCodecFolderList
	
	#for encoder command 
	declare -a EncoderCommandSet
	declare -a EncoderCommandName
	declare -a EncoderCommandValue
	
	#encoder parameters  change based on the case info
	CaseInfo=""
	BitStreamFile=""
	RecYUVFile=""
	DiffInfo=""
	DiffFlag=""
	BisttreamBenchmark=""
	BisttreamTarget=""
	
	#pass number
	TotalCaseNum=""
	PassCaseNum=""
	UnpassCaseNum=""

}


#called by runGlobalVariableInitial
#usage  runGenerateSequencePath 
#output  YUVpath
runGenerateSequencePath()
{

	if [[ "$TestSetIndex" =~ CIF$ ]]
	then
		SequencePath=${TestSequencePath}/CIF
	elif [[  "$TestSetIndex" =~ VGA$ ]]
	then
		SequencePath=${TestSequencePath}/VGA
	elif [[   "$TestSetIndex" =~ XGA$ ]]
	then
		SequencePath=${TestSequencePath}/XGA
	elif [[  "$TestSetIndex" =~ 720p$ ]]
	then
		SequencePath=${TestSequencePath}/720p
	elif [[  "$TestSetIndex" =~ 1080p$ ]]
	then
		SequencePath=${TestSequencePath}/1080p
	elif [[  "$TestSetIndex" =~ HEVC$  ]]
	then
		SequencePath=${TestSequencePath}/HEVC
	elif [[  "$TestSetIndex" =~ Special$  ]]
	then
		SequencePath=${TestSequencePath}/Special
	fi	
	
	SequencePath=${SequencePath}/${TestSequenceName}
		
	if [ ! -e ${SequencePath}  ]
	then
		echo "${SequencePath} can not be found!"
		exit 1
	else
		echo "sequence path is ${SequencePath} "
	fi
	
}

#called by runGlobalVariableInitial
#usage runGetYUVInfo  $TestSequencName
#eg. input:  ABC_1920X1080_30fps_XXX.yuv  output: 1920 1080 30
#eg. input:  ABC_1920X1080_XXX.yuv        output: 1920 1080 0
#eg. input:  ABC_XXX.yuv                  output: 0    0    0
runGetYUVInfo()
{
	if [ ! $# -eq 1  ]
	then 
		echo "no parameter!"
		return 1
	fi
	
	local SequenceName=$1
	local PicWidth="0"
	local PicHeight="0"
	local FPS="0"
	
	declare -a aPicInfo
	aPicInfo=(`echo ${SequenceName} | awk 'BEGIN {FS="[_.]"} {for(i=1;i<=NF;i++) printf("%s  ",$i)}'`)
	
	local Iterm
	local Index=""
	local Pattern_01="[xX]"
	local Pattern_02="^[1-9][0-9]"
	local Pattern_03="[1-9][0-9]$"
	local Pattern_04="fps$"
	#get PicW PicH info
	let "Index=0"
	for  Iterm in ${aPicInfo[@]}
	do
		if [[ $Iterm =~ $Pattern_01 ]] && [[ $Iterm =~ $Pattern_02 ]] && [[ $Iterm =~ $Pattern_03 ]]
		then
			PicWidth=`echo $Iterm | awk 'BEGIN {FS="[xX]"} {print $1}'`
			PicHeight=`echo $Iterm | awk 'BEGIN {FS="[xX]"} {print $2}'`
			break
		fi
		let "Index++"
	done
	
	#get fps info
	let "Index++"
	if [ $Index -le ${#aPicInfo[@]} ] 
	then
		if [[ ${aPicInfo[$Index]} =~ ^[1-9]  ]] || [[ ${aPicInfo[$Index]} =~ $Pattern_04 ]]
		then
			FPS=`echo ${aPicInfo[$Index]} | awk 'BEGIN {FS="[a-zA-Z]" } {print $1} '`
		fi	
	fi
	echo "$PicWidth $PicHeight $FPS"
}


#called by runGlobalVariableInitial
#usage runEncoderCommandInital
runEncoderCommandInital()
{
	EncoderCommandSet=(-frms   \
					   -gop    \
					   -scrsig \
					   -rc     \
					   -rcm    \
					   -tarb   \
					   "-lqp 0"    \
					   -cabac      \
					   "-slcmd 0"  \
					   "-slcnum 0" \
					   -thread \
					   -ltr    \
					   -db     \
					   "-slcsize 0")
	EncoderCommandName=(FrEcoded   \
						GOPSize    \
						ContentSig \
						RC         \
						RCMd       \
						BitRate    \
						QP         \
						CABAC      \
						SlcMd      \
						SlcMum     \
						ThrMum     \
						LTR        \
						LFilterIDC \
						SlcSize)
	EncoderCommandValue=(0 0 0 0 0   0 0 0 0 0  0 0 0 0)
	NumParameter=${#EncoderCommandSet[@]}
	
}	

runGlobalVariableInitial()
{

    #TestDataSpaceDir=../AllTestData  CurrentDir=../AllTestData/TestSetXXX/***.yuv   eg ../AllTestData/TestSetCIF/basketball.yuv
	#WorkingDir folder include   ./AllTestData   ./result  ./bats  ./cfg
	CurrentDir=`pwd`
	cd ..
	TestSetDir=`pwd`
	cd ../../
	WorkDir=`pwd`
	#go back to current dir
	cd ${CurrentDir}
	
	#test data space
	FinalResultPath="${TestSetDir}/result"
	IssueDataPath="${TestSetDir}/issue"
	TempDataPath="${CurrentDir}/TempData"
	mkdir -p ${TempDataPath}
	
	#for test sequence info
	TestSequencePath="/opt/TestSequence"
	TestSetIndex=`echo $CurrentDir | awk 'BEGIN {FS="/"} { i=NF-1; print $i}'`
	TestSequenceName=`echo $CurrentDir | awk 'BEGIN {FS="/"} { i=NF; print $i}'`
	#generate YUV path  $SequencePath
	runGenerateSequencePath
	#get YUV detail info $picW $picH $FPS
	declare -a aYUVInfo
	aYUVInfo=(`runGetYUVInfo  ${TestSequenceName}`)
	PicW=${aYUVInfo[0]}
	PicH=${aYUVInfo[1]}

	#test cfg file and test info output file
	ConfigureFile=welsenc.cfg
	AllCaseFile=${TestSetIndex}_${TestSequenceName}.csv
	AllCasePassStatusFile="${FinalResultPath}/${TestSetIndex}_${TestSequenceName}_AllCaseOutput.csv"	
	echo	"BitMatched Status,           \
			-frms, -gop, -scrsig, -rc,    \
			-rcm, -tarb, -lqp 0, -cabac,  \
			-slcmd 0,-slcnum 0, -thread,  \
			-ltr, -db, -slcsize 0 ">${AllCasePassStatusFile}
	
	#intial Commandline parameters
	runEncoderCommandInital
	
	
	let "TotalCaseNum=0"
	let "PassCaseNum=0"
	let "UnpassCaseNum=0"

	
	#initial codec folder 
	declare  -a aCodecFolderList
	aCodecFolderList=(codec_benchmark  codec_target)
	echo "codec folder is ${aCodecFolderList[@]}"
	local codec=""
	for codec in ${aCodecFolderList[@]}
	do
		if [  ! -e ${codec} ]
		then 
			echo " ${codec} is unavailable"
			exit 1
		fi
	done
	
	
}


#***********************************************************
#call by  runAllCaseTest
# parse case info --encoder preprocess
#usage  runGetEncoderCommandValue $CaseData
runParseCaseInfo()
{

	if [ $#  -lt 1  ]
	then 
		echo "no parameter!"
		return 1
	fi

    local TempData=""
	local BitstreamPrefix=""
	local CaseData=$@
	
	TempData=`echo $CaseData |awk 'BEGIN {FS="[,\r]"} {for(i=1;i<=NF;i++) printf(" %s",$i)} ' `
	EncoderCommandValue=(${TempData})

	for((i=0; i<$NumParameter; i++))
	do
		BitstreamPrefix=${BitstreamPrefix}_${EncoderCommandName[$i]}_${EncoderCommandValue[$i]}
	done
  
	
	BitstreamBenchmark=${TempDataPath}/${TestSequenceName}_${BitstreamPrefix}_codec_benchmark.264
	BitstreamTarget=${TempDataPath}/${TestSequenceName}_${BitstreamPrefix}_codec_target.264
	RecYUVFile=${TempDataPath}/${TestSequenceName}_${BitstreamPrefix}_rec.yuv
	DiffInfo=${TempDataPath}/${TestSequenceName}_${BitstreamPrefix}_diff.info
		
	echo ""
	echo "BitstreamPrefix is ${BitstreamPrefix}"
	echo ""

}


#call by  runAllCaseTest
#usage  runEncodeOneCase  $CodecFolder  
runEncodeOneCase()
{
	if [ $#  -lt 1  ]
	then 
		echo "no parameter!"
		return 1
	fi
	
	local CodecDir=$1
	local TempDir=""
	local CaseInfo=""
	
	cd  ${CodecDir}
	TempDir=`pwd`
	
	#rename the bitstream file and yuv file based on the codec version
	if [[ ${CodecFolder} =~ codec_benchmark  ]]
	then
		BitStreamFile=${BitstreamBenchmark}
	else
		BitStreamFile=${BitstreamTarget}
	fi
	
	
	CaseCommand="${ConfigureFile}  	   \
		-numl 1 layer2.cfg   		   \
		-sw 0 ${PicW} -sh 0 ${PicH}    \
		-iper 240                      \
		${EncoderCommandSet[0]}  ${EncoderCommandValue[0]}  \
		${EncoderCommandSet[1]}  ${EncoderCommandValue[1]}  \
		${EncoderCommandSet[2]}  ${EncoderCommandValue[2]}  \
		${EncoderCommandSet[3]}  ${EncoderCommandValue[3]}  \
		${EncoderCommandSet[4]}  ${EncoderCommandValue[4]}  \
		${EncoderCommandSet[5]}  ${EncoderCommandValue[5]}  \
		${EncoderCommandSet[6]}  ${EncoderCommandValue[6]}  \
		${EncoderCommandSet[7]}  ${EncoderCommandValue[7]}  \
		${EncoderCommandSet[8]}  ${EncoderCommandValue[8]}  \
		${EncoderCommandSet[9]}  ${EncoderCommandValue[9]}  \
		${EncoderCommandSet[10]} ${EncoderCommandValue[10]} \
		${EncoderCommandSet[11]} ${EncoderCommandValue[11]} \
		${EncoderCommandSet[12]} ${EncoderCommandValue[12]} \
		${EncoderCommandSet[13]} ${EncoderCommandValue[13]} "	
	
	echo ""
	echo "case line is :"
	echo "${CaseCommand}"
	
	./welsenc.exe   ${CaseCommand}    		  \
					-bf   ${BitStreamFile}    \
					-org  0 ${SequencePath}   \
					-drec 0 ${RecYUVFile}     
		

	echo ""
	cd ${CurrentDir}

}


#call by  runAllCaseTest  $BenchmarkFolder $TargetFolder 
#bitstream compare
#usage  runMultiCodecCompare
runMultiCodecCompare()
{
	if [ ! $#  -eq 2  ]
	then 
		echo "no parameter!"
		return 1
	fi

	local BenchmarkFolder=$1
	local TargetFolder=$2
	
	echo ""
	echo "******************************************"
	echo "diff"
	diff -q ${BitstreamBenchmark}   ${BitstreamTarget}>${DiffInfo}
		
	if [  -s ${DiffInfo} ]
	then 
		echo "diff info:  bitsteam not matched "
		cp -f ${BitstreamBenchmark}         ${IssueDataPath}/${BitStreamFile}
		cp -f ${BitstreamTarget}            ${IssueDataPath}/${BitStreamFile}_diff
		DiffFlag="1:unpassed!"
		let "UnpassCaseNum++"
		
	else
		echo "bitstream pass"
		DiffFlag="0:passed!"
		let "PassCaseNum++"
	fi	
	
}

#call by  runAllCaseTest
#delete needless files and output single case test result to log file
#usage  runSingleCasePostAction $CaseData
#usage runPostAction  $CaseData
runSingleCasePostAction()
{
	if [ $#  -lt 1  ]
	then 
		echo "no parameter!"
		return 1
	fi
	
	local CaseData=$@
	
	CaseInfo=`echo $CaseData | awk 'BEGIN {FS="[,\r]"} {for(i=1;i<=NF;i++) printf(" %s,",$i)} '` 
	echo "${DiffFlag}, ${CaseInfo}">>${AllCasePassStatusFile}
	
	rm -f ${DiffInfo}
	rm -f ${TempDataPath}/*

}

#usage runOutputPassNum
runOutputPassNum()
{
	# output file locate in ../result
	echo ""
	echo "***********************************************************"
	echo "${TestSetIndex}_${TestSequenceName}"
	echo "total case  Num is :  ${TotalCaseNum}"
	echo "pass  case  Num is : ${PassCaseNum}"
	echo "unpass case Num is : ${UnpassCaseNum} "
	echo "issue bitstream can be found in .../AllTestData/${TestSetIndex}/issue"
	echo "detail result  can be found in .../AllTestData/${TestSetIndex}/result"
	echo "***********************************************************"
	echo ""
}


# run all test case based on XXXcase.csv file
#usage  runAllCaseTest
runAllCaseTest()
{
	while read CaseData
	do
		#get case parameter's value 
		if [[ $CaseData =~ ^[0-9]  ]]
		then			
			echo ""
			echo ""
			echo ""
			echo "********************case index is ${TotalCaseNum}**************************************"			
			runParseCaseInfo ${CaseData}
			echo ""
	
			#initial codec folder 
			aCodecFolderList=(codec_benchmark  codec_target)
			for CodecFolder in ${aCodecFolderList[@]}
			do
				echo "******************************************"
				echo "folder is ${CodecFolder}"
				runEncodeOneCase  ${CodecFolder}				
			done
			
			runMultiCodecCompare     ${aCodecFolderList[@]}
			runSingleCasePostAction  ${CaseData}
			
			let "TotalCaseNum++"				
		fi
	done <$AllCaseFile
	
	runOutputPassNum

}


#***********************************************************
runMain()
{

	runGlobalVariableDef
	runGlobalVariableInitial
	
	AllCaseConsoleLogFile="../result/${TestSetIndex}_${TestSequenceName}.TestLog"
	CaseSummaryFile="../result/${TestSetIndex}_${TestSequenceName}.Summary"
	FlagFile=""
	
	#run all cases
	runAllCaseTest>${AllCaseConsoleLogFile}
	
	
	# output file locate in ../result
	echo "${TestSetIndex}_${TestSequenceName}, \
		 ${PassCaseNum} pass!,                  \
		 ${UnpassCaseNum} unpass!,				\
		 detail file located in ../AllTestData/${TestSetIndex}/result">${CaseSummaryFile}
	
	#generate All case Flag
	if [  ! ${UnpassCaseNum} -eq 0  ]	
	then
		FlagFile="../result/${TestSetIndex}_${TestSequenceName}.unpassFlag"
	else
		FlagFile=" ../result/${TestSetIndex}_${TestSequenceName}.passFlag"
	fi
	touch ${FlagFile}
	
	runOutputPassNum
	
	#prompt info
	echo ""
	echo "***********************************************************"
	echo "detail file include:"
	echo "../AllTestData/${TestSetIndex}/result/  ${TestSetIndex}_${TestSequenceName}_AllCaseOutput.csv"
	echo "../AllTestData/${TestSetIndex}/result/  ${AllCaseConsoleLogFile}"
	echo "../AllTestData/${TestSetIndex}/result/  ${CaseSummaryFile}"
	echo "../AllTestData/${TestSetIndex}/result/  ${FlagFile}"
	echo "***********************************************************"
	echo ""
	
}


#call main function 
runMain


