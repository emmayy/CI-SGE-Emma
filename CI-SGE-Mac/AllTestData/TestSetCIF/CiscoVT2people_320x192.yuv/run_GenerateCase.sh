#!/bin/bash


echo "************usage***********"
echo " ./run_*.sh  case.cfg OutputCaseFile  TestSetIndex:CIF VGA XGA 720p 1080p HEVC  SetSpecial"
echo "****************************"

#
configFile=$1
casefile=$2
testSet=$3   # CIF VGA XGA 720p 1080p HEVC

runGlobalVariableInital()
{
	BitRatePattern="TargetBitrate${testSet}"

	let " FramesToBeEncoded = 0"
	let " GOPSize   = 0"
	let " SliceSize = 0"

	declare -a  ScreenContentSignal
	declare -a  EnableRC
	declare -a  RCMode
	declare -a  TargetBitrate
	declare -a  InitialQP

	declare -a  EntropyCodingMode
	declare -a  SliceMode
	declare -a  SliceNum
	declare -a  MultipleThreadIdc

	declare -a  EnableLongTermReference
	declare -a  LoopFilterDisableIDC 

	#generate test cases and output to case file
	casefile_01=${casefile}_01.csv
	casefile_02=${casefile}_02.csv
	casefile_03=${casefile}_03.csv
	casefile_04=${casefile}_04.csv

}

#usage runParseCaseConfigure $ConfigureFile
runParseCaseConfigure()
{
	#parameter check! 
	if [ ! $# -eq 1  ]
	then
		echo "need parameters for this function!"
		return 1
	 fi

	local ConfigureFile=$1

	#read configure file
	while read line
	do
		command=$line #`echo $line | awk 'BEGIN {FS="[#:]"} {print $1}'`
		if [[ "$command" =~ ^FramesToBeEnc  ]]
		then
			FramesToBeEncoded=`echo $line | awk 'BEGIN {FS="[#:]" } {print $2}' `
			
		elif [[ "$command" =~ ^ScreenContentSignal ]]
		then
			ScreenContentSignal=`echo $line | awk 'BEGIN {FS="[#:]"} {print $2}' `
			
		elif [[ "$command" =~ ^EntropyCoding ]]
		then
			EntropyCodingMode=`echo $line | awk 'BEGIN {FS="[#:]"} {print $2}' `
			
		elif [[ "$command" =~ ^GOPSize ]]
		then
			GOPSize=`echo $line | awk 'BEGIN {FS="[#:]"} {print $2}' `
			
		elif [[ "$command" =~ ^MultipleThreadIdc ]]
		then
			MultipleThreadIdc=`echo $line | awk 'BEGIN {FS="[#:]"} {print $2}' `
			
		elif [[ "$command" =~ ^SliceMode ]]
		then
			SliceMode=`echo $line | awk 'BEGIN {FS="[#:]"} {print $2}' `
			
		elif [[ "$command" =~ ^SliceSize ]]
		then
			SliceSize=`echo $line | awk 'BEGIN {FS="[#:]"} {print $2}' `
			
		elif [[ "$command" =~ ^SliceNum ]]
		then
			SliceNum=`echo $line | awk 'BEGIN {FS="[#:]"} {print $2}' `
			
		elif [[ "$command" =~ ^EnableRC ]]
		then
			EnableRC=`echo $line | awk 'BEGIN {FS="[#:]"} {print $2}' `
			
		elif [[ "$command" =~ ^RCMode ]]
		then
			RCMode=`echo $line | awk 'BEGIN {FS="[#:]"} {print $2}' `
			
		elif [[ "$command" =~ ^"$BitRatePattern" ]]
		then
			TargetBitrate=`echo $line | awk 'BEGIN {FS="[#:]"} {print $2}' `
			
		elif [[ "$command" =~ ^EnableLongTermReference ]]
		then
			EnableLongTermReference=`echo $line | awk 'BEGIN {FS="[#:]"} {print $2}' `
			
		elif [[ "$command" =~ ^LoopFilterDisableIDC ]]
		then
			LoopFilterDisableIDC=`echo $line | awk 'BEGIN {FS="[#:]"} {print $2}' `
		
		elif [[ "$command" =~ ^InitialQP ]]
		then
			InitialQP=`echo $line | awk 'BEGIN {FS="[#:]"} {print $2}' `
		fi

	done <$ConfigureFile

}


#the first stage for case generation
runFirstStageCase()
{
	for ScreenSignal in ${ScreenContentSignal[@]}
	do
		for RCFlag in ${EnableRC[@]}
		do
			## set RCMode QP and BitRate value based on RCFlag
			
			declare -a RCModeforTest
			declare -a QPforTest
			declare -a BitRateforTest
			if [ $RCFlag -eq 0 ]
			then
				RCModeforTest=(0)
				QPforTest=(24 36)
				BitRateforTest=(256)			
			else
				RCModeforTest=${RCMode[@]}
				QPforTest=(26)
				BitRateforTest=${TargetBitrate[@]}			
			fi
		
			#......for loop.........................................#
			for RCModeIndex in ${RCModeforTest[@]}
			do
				for QPIndex in ${QPforTest[@]}
				do
					for BitRateIndex in ${BitRateforTest[@]}
					do				
						echo   "$FramesToBeEncoded,\
								$GOPSize, \
								$ScreenSignal,\
								$RCFlag,\
								$RCModeIndex,\
								$BitRateIndex,\
								$QPIndex">>$casefile_01						
					done # BitRate loop
				done # QP loop
							
			done #RCMode loop	    	
		done #RCFlag loop
		#..........................................................#
	done

}


##second stage for case generation
runSecondStageCase()
{
	while read FirstStageCase
	do
		for EntropyIndex in ${EntropyCodingMode[@]}
		do
			for SlcMode in ${SliceMode[@]}
			do  
				#for slice number based on different sliceMode
				declare -a SliceNumber
				declare -a ThreadNumber
				
				if [ $SlcMode -eq 1 ]
				then
					SliceNumber=(4 7)
				else
					SliceNumber=(0)
				fi
				#for slice number based on different thread number	
				if [ $SlcMode -eq 0 ]
				then 
					ThreadNumber=(1)
				else
					ThreadNumber=${MultipleThreadIdc[@]}
				fi
				
				for SlcNum in ${SliceNumber[@]}
				do
					for ThreadNum in ${ThreadNumber[@]}
					do
						if [[ $FirstStageCase =~ ^[0-9] ]]
						then
							echo   "$FirstStageCase,\
									$EntropyIndex,\
									$SlcMode, \
									$SlcNum,\
									$ThreadNum">>$casefile_02
						fi
					done #threadNum loop		    	
				done #sliceNum loop
			done #Slice Mode loop
		done # Entropy loop
	done <$casefile_01
	
}

#the third stage for case generation
runThirdStageCade()
{
	while read SecondStageCase
	do
		for LTRFlag in ${EnableLongTermReference[@]}
		do
			for LoopfilterIndex in ${LoopFilterDisableIDC[@]}
			do
				if [[ $SecondStageCase =~ ^[0-9] ]]
				then
					echo "$SecondStageCase,\
						  $LTRFlag,\
						  $LoopfilterIndex,\
						  $SliceSize">>$casefile
				fi		
			done
		done
	done <$casefile_02

}

#only for test
runOutputParseResult()
{
	echo "all case info has been  output to file $casefile "

	echo "Frames=     $FramesToBeEncoded"
	echo "GOPsize=    $GOPSize"
	echo "SliceSize=  $SliceSize"
	echo "ScreenContentSignal= ${ScreenContentSignal[@]}"
	echo "EnableRC=         ${EnableRC[@]}"
	echo "RCMode=           ${RCMode[@]}"
	echo "TargetBitrate=    ${TargetBitrate[@]}"
	echo "InitialQP=        ${InitialQP[@]}"

	echo "EntropyCodingMode= ${EntropyCodingMode[@]}"
	echo "SliceMode=         ${SliceMode[@]}"
	echo "SliceNum=          ${SliceNum[@]}"
	echo "MultipleThreadIdc= ${MultipleThreadIdc[@]}"

	echo "EnableLongTermReference=${EnableLongTermReference[@]}"
	echo "LoopFilterDisableIDC=   ${LoopFilterDisableIDC[@]}"

}

runBeforeGenerate()
{
	headline="FramesToBeEncoded,\
					GOPSize, \
					ScreenContentSignal,\
					EnableRC,\
					RCMode,\
					TargetBitrate,\
					InitialQP,\
					EntropyCodingMode,\
					SliceMode,\
					SliceNum,\
					MultipleThreadIdc,\
					EnableLongTermReference,\
					LoopFilterDisableIDC,\
					SliceSize" 

	echo $headline>$casefile			
	echo $headline>$casefile_01
	echo $headline>$casefile_02
	echo $headline>$casefile_03
	echo $headline>$casefile_04

}


runAfterGenerate()
{
	# deleted temp_case file
	rm -f $casefile_01
	rm -f $casefile_02
	rm -f $casefile_03
	rm -f $casefile_04
}

runMain()
{
	runGlobalVariableInital 
	runBeforeGenerate
	runParseCaseConfigure  ${configFile}
	runOutputParseResult
	
	runFirstStageCase
	runSecondStageCase
	runThirdStageCade
	
	runAfterGenerate
}

#
echo ""
echo "case generating ......"
runMain







