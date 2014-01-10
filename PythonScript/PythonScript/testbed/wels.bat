echo on
REM  This is the dos version batch file for WELS decoder
REM  Please set the sequence number you want to test
REM  and enumerate all the sequence files following the total number
cls
echo "the tests of WELS decoder started here"
date /T
time /T

set sequence_number=6

set sequence[1]="foreman_cif_30fps_I_nodeblocking"
set sequence[2]="foreman_cif_30fps_IDR"
set sequence[3]="foreman_cif_30fps_IDR_I16x16_nodeblocking"
set sequence[4]="foreman_cif_30fps_IPP_nodeblocking"
set sequence[5]="foreman_IDR"
set sequence[6]="foreman_qcif_15fps_20frames"



set index=1
FOR /L %%x IN (1,1,%sequence_number%) DO test_core.bat

time /T
echo "this is the end of tests"
REM the test is over, please check the test results
