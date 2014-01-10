#include <stdlib.h>
#include "WelsTrace.h"
#include "WelsOSAbstract.h"
#include "Outgoing.h"
#include "CacheManager.h"
#include "InputBitstream.h"

#define  BUF_SIZE 1000
using namespace WelsUtilLib;

void DumpData(unsigned char * pData, int nLen, unsigned int uiTimeStamp, FILE* pFile)
{
	static uint32_t ulLastTimestamp = -1;
	static uint32_t ulStartCurTime  = -1;
	static uint32_t ulStartTimestamp  = -1;
	static uint32_t uiDumpedDataLength  = 0;
	uint32_t ulCurTime, ulTimeStampDelta, ulCurTimeDelta, ulDelayDelta, uLastTimeStampDelta, ulLastDelayDelta;
    static int32_t iFrameCount = 0;

	if (-1 == ulStartCurTime)
	{
		ulStartCurTime = CWelsOSAbstract::WelsNow();
		ulStartTimestamp = uiTimeStamp;
	}

	if (ulLastTimestamp != uiTimeStamp)
	{	
		FILE* pFileOutputLog = fopen("switch_output.log","a+");

		ulCurTime = CWelsOSAbstract::WelsNow();

		ulTimeStampDelta = WELS_ABS(uiTimeStamp - ulStartTimestamp);
		ulCurTimeDelta = WELS_ABS(ulCurTime-ulStartCurTime);
		ulDelayDelta = WELS_ABS(ulTimeStampDelta-ulCurTimeDelta);

        if (-1 != ulLastTimestamp)
        {
        uLastTimeStampDelta = WELS_ABS(ulLastTimestamp - ulStartTimestamp);
		ulLastDelayDelta = WELS_ABS(uLastTimeStampDelta-ulCurTimeDelta);
        }
        else
            uLastTimeStampDelta = ulLastDelayDelta = 0;

        iFrameCount++;
		WELS_INFO_TRACE("Dump Data Log at Beginning of Frames, TimeStamp Delta:\t"<<ulTimeStampDelta<<"\t, ulCurTime Delta:\t"<<ulCurTimeDelta<<"\t, Timestamp and CurTime Delta:\t"<<ulDelayDelta<<"\t, CurTime and LastTimestamp Delta:\t"<<ulLastDelayDelta<<"\t,DeliverData(B)InTimeDelta:\t"<<((ulCurTimeDelta*200)>>3)<<",DeliverData(B)"<<uiDumpedDataLength);
		fprintf(pFileOutputLog,"Dump Data Log at Beginning of Frame(%d), TimeStamp Delta:\t%d\t, ulCurTime Delta:\t%d\t, Timestamp and CurTime Delta:\t%d\t, CurTime and LastTimestamp Delta:\t%d\nDump Data Log, DeliverDataInTimeDeltaInByte:\t%d\t, DeliverDataInByte:\t%d\t\n", 
			iFrameCount, ulTimeStampDelta, ulCurTimeDelta,  ulDelayDelta, ulLastDelayDelta, ((ulCurTimeDelta*200)>>3), uiDumpedDataLength );

		fclose(pFileOutputLog);

		ulLastTimestamp = uiTimeStamp;
	}

	uiDumpedDataLength+=nLen;

	// TODO,  by Sijia
	if (NULL!=pFile)
	{
		fwrite(pData, sizeof(unsigned char), nLen, pFile);
	}
}

int main( int argc, char **argv )
{	
	if (argc < 2)
	{
		WELS_ERROR_TRACE("Command lines parse failed!");
		return 1;
	}

	IWelsTrace::Init(IWelsTrace::WELS_TRACE_WBX, NULL);
    WELS_INFO_TRACE("Begin Switch Testing");

	CInputBitstream cInputBitstream(argv[1], argv[2], argv[3]);
	if (!cInputBitstream.ReadBitstreamIntoList())
	{
		WELS_ERROR_TRACE("Cannot read src successfully!");
	}


	//----
	//init Cache
	unsigned char  buf[BUF_SIZE];
	memset(buf, 0, BUF_SIZE);

	COutgoing   cOutgoing; 
    SwitchCacheHandle   h = CreateSwitchHandle(&cOutgoing);

	cOutgoing.SetCache(h, DumpData, cInputBitstream.GetOutputFile());

	DWORD  dwUserID = 0;
	Subscribe(h, dwUserID, true);
	cOutgoing.Subscribe(dwUserID, true,  150000);
	//----


	//----
	//Getting  data ready
	int  nFrameCount = cInputBitstream.GetAUTotalNum();

	//----
	//Cache's timer begins
	StartCache();
//#define FIXED_TOTAL_TIME

#ifdef FIXED_TOTAL_TIME
#define TIMERTOTALCOUNT (4000)//1000 takes about 32seconds
#endif 

	MSG  msg; 
	unsigned short uSeq = 0;
	unsigned int  ulCurTime=0, ulFirstTime=0, ulTimestamp=0, ulFirstTimestamp=0;
	int  iPacketTotalCount = 0, iIdx =0, iPacketCount=0;
	int  iTotalDataLen = 0;

#ifdef FIXED_TOTAL_TIME
    int  iTimerCount = 0;
    while( GetMessage(&msg, 0, 0, 0) && (iTimerCount<TIMERTOTALCOUNT) ){
#else
	while( GetMessage(&msg, 0, 0, 0) && (iIdx<nFrameCount) ){
#endif

        ulCurTime = CWelsOSAbstract::WelsNow();
        ulTimestamp = cInputBitstream.GetCurrentAUTime(iIdx);

		if (iIdx<nFrameCount) 
		{
			if ( (iIdx == 0) || 
				(WELS_ABS(ulCurTime-ulFirstTime) >= WELS_ABS(ulTimestamp - ulFirstTimestamp)) )
			{
                //WELS_INFO_TRACE("Current Input Frame: "<<iIdx<<", ulTimestamp= "<<ulTimestamp<<
                  //  " ulCurTime= "<<ulCurTime<<", ulFirstTime= "<<ulFirstTime<<
                    //" ulCurTime-ulFirstTime: "<<WELS_ABS(ulCurTime-ulFirstTime));

                if (iIdx == 0)
                {
                    ulFirstTime = ulCurTime;
                    ulFirstTimestamp = ulTimestamp;
                }
				iTotalDataLen += cInputBitstream.GetCurrentAUSize(iIdx);

				iPacketCount = cInputBitstream.GetCurrentAUPacketNum(iIdx, 1200);//TODO

				for( int i=0;i<iPacketCount;i++ ){

					ReceiveData(h, 
						cInputBitstream.GetCurrentPacketPointer(i),//buf, 
						cInputBitstream.GetCurrentPacketLen(i),//BUF_SIZE, 
						cInputBitstream.GetCurrentAUTime(iIdx),//ulTimestamp,
						cInputBitstream.GetCurrentAUPriority(iIdx),//0, 
						++uSeq);
				}

				iPacketTotalCount+=iPacketCount;

				++iIdx;

   


			}
		}

		CCacheManager::Instance().SendCacheData();
#ifdef FIXED_TOTAL_TIME
        iTimerCount++;
#endif
	}

	
	{
		FILE* pFileOutputLog = fopen("switch_output.log","a+");
		WELS_INFO_TRACE("Dump Data Log, TotalInputData:\t"<<iTotalDataLen);
		fprintf(pFileOutputLog,"Dump Data Log, TotalInputData:\t%d\t", iTotalDataLen);
		fclose(pFileOutputLog);
	}

	WELS_INFO_TRACE("NETWORKSINK:  Total Revcive Packet "<<iPacketTotalCount);
	StopCache();
	//---- cache timer ends

	DestroySwitchCache(h);

	return 0;
}

