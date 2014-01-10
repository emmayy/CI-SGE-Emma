REM the module to decode the standard stream with different decoders
REM and compare their output of YUV file

set yuvfile[1]=
set yuvfile[2]=

echo index=%index%

if %index% == 1 set sequencefile=%sequence[1]%
if %index% == 2 set sequencefile=%sequence[2]%
if %index% == 3 set sequencefile=%sequence[3]%
if %index% == 4 set sequencefile=%sequence[4]%
if %index% == 5 set sequencefile=%sequence[5]%
if %index% == 6 set sequencefile=%sequence[6]%

echo sequencefile=%sequencefile%

copy /Y %sequencefile% test.264

REM decode the stream with JM decoder
REM JMdecoder(JM98) input:test.264, output:test.yuv

ldecod.exe

set decodernum=1
REM save the YUV file decoded by JM decoder
set outputyuvfile=%sequencefile:264=yuv%
set outputyuvfile=%outputyuvfile%_%decodernum%
echo %outputyuvfile%
set yuvfile[%decodernum%]=%outputyuvfile%
ren test_dec.yuv %outputyuvfile%

REM decode same stream with with wels deocder
REM Wels decoder, input: test.264, output:test.yuv

#wels decoder.exe test.264 test.yuv
decConsole.exe test.264 test.yuv

set decodernum=2
set outputyuvfile=%sequencefile:264=yuv%
set outputyuvfile=%outputyuvfile%_%decodernum%
echo %outputyuvfile%
set yuvfile[%decodernum%]=%outputyuvfile%
ren test_dec.yuv %outputyuvfile%

REM compare the YUV bits between two output streams
set testresultfile=%sequencefile:264=res%
echo test result report file is %testresultfile%

echo fc /B %yuvfile[1]% %yuvfile[2]% > %testresultfile%
fc /B %yuvfile[1]% %yuvfile[2]% > %testresultfile%
set /A index+=1


