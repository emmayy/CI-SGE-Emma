
#include "InputBitstream.h"

//#define WITHONLINEBS

CInputBitstream::CInputBitstream( const char *sBitstreamName, const char *sIdcFileName, const char *sOutputBitstreamName )
: m_pBitstream(0)
, m_pIdcFile(0)
, m_pOutputBitstream(0)
{
	if ( strlen(sBitstreamName) > 0 ){	// confirmed_safe_unsafe_usage
		m_pBitstream = fopen(sBitstreamName, "rb");
	}
	if ( strlen(sIdcFileName) > 0 ){	// confirmed_safe_unsafe_usage
		m_pIdcFile = fopen(sIdcFileName, "r");
	}
	if ( strlen(sOutputBitstreamName) > 0 ){	// confirmed_safe_unsafe_usage
		m_pOutputBitstream = fopen(sOutputBitstreamName, "wb");
	}
}

CInputBitstream::CInputBitstream( const string& sBitstreamName, const string& sIdcFileName, const string& sOutputBitstreamName )
: m_pBitstream(0)
, m_pIdcFile(0)
, m_pOutputBitstream(0)
{
	if ( sBitstreamName.length() > 0 )
	{
		m_pBitstream = fopen(sBitstreamName.c_str(), "rb");
	}
	if ( sIdcFileName.length() > 0 )
	{
		m_pIdcFile = fopen(sIdcFileName.c_str(), "r");
	}	
	if ( sOutputBitstreamName.length() > 0 )
	{
		m_pOutputBitstream = fopen(sOutputBitstreamName.c_str(), "wb");
	}
}

CInputBitstream::~CInputBitstream()
{
	if ( m_pBitstream ){
		fclose( m_pBitstream );
		m_pBitstream = NULL;
	}
	if ( m_pIdcFile ){
		fclose( m_pIdcFile );
		m_pIdcFile = NULL;
	}
	if ( m_pOutputBitstream ){
		fclose( m_pOutputBitstream );
		m_pOutputBitstream = NULL;
	}

	if (m_pInputBsBuffer)
		WelsSafeFree(m_pInputBsBuffer);
	if (m_pAUPointerList)
		WelsSafeFree(m_pAUPointerList);
	if (m_pAuLenList)
		WelsSafeFree(m_pAuLenList);
	if (m_pFrameIdcList)
		WelsSafeFree(m_pFrameIdcList);
	if (m_pTimestampList)
		WelsSafeFree(m_pTimestampList);

}

bool CInputBitstream::ReadBitstreamIntoList()
{
	int32_t iCurPos = 0;
	int32_t iAuLen = 0;
    int32_t iIdx, iLen =0, iFrameIdc;
    	
    if (NULL==m_pBitstream)
	{
		return false;
	}
	//read in m_pBitstream
	fseek(m_pBitstream, 0, SEEK_END);
	m_iBitstreamLength = ftell(m_pBitstream);
	m_pInputBsBuffer = new BS_Element[m_iBitstreamLength+1];
	if (NULL==m_pInputBsBuffer)
	{
		return false;
	}
	fseek(m_pBitstream, 0, SEEK_SET);
	if ( fread(m_pInputBsBuffer, sizeof(BS_Element), m_iBitstreamLength, m_pBitstream )  < m_iBitstreamLength*sizeof(BS_Element) )
	{
		return false;
	}

	//find au len
	m_pAUPointerList = static_cast<BS_Element**>(WelsMalloc(m_iBitstreamLength>>2));
	m_pAuLenList = static_cast<int*>(WelsMalloc(m_iBitstreamLength>>2));
	m_pAuLenListLength = 0;
    int32_t slice_count = 0, slice_num=8;
	while (iCurPos < m_iBitstreamLength)
	{
		iLen = FindNextNalHeader(iCurPos);
		iAuLen += iLen;
		iCurPos+= iLen;
#ifdef WITHONLINEBS
        if (m_bIsTheNalVcl) slice_count ++;

		if ( (m_bIsTheNalVcl && (slice_count==slice_num+1) && !m_bIsTheNalIdr)
#else
        if ( (m_bIsTheNalVcl && !m_bIsTheNalIdr)
#endif
			|| m_bIsTheNalSps 
			|| (iCurPos == m_iBitstreamLength))
		{
            slice_count = 0;
			//one AU finish
			iAuLen=AddOneAU(iAuLen);
		}
	}


	//fill in Idc
	m_pFrameIdcList = static_cast<int*>(WelsMalloc(m_iBitstreamLength>>2));
	m_pTimestampList = static_cast<int*>(WelsMalloc(m_iBitstreamLength>>2));

	iIdx =0;
	int iTimeStamp =0, iSizeInBytes =0, iSizeInBits = 0;
    if (NULL == m_pIdcFile)
        return false;
    bool bNeedSizeReset = false;



	while (!feof(m_pIdcFile))
	{
		iFrameIdc =0;
		//fread( &iFrameIdc, sizeof(int), 1, m_pIdcFile ); 
		fscanf(m_pIdcFile, "%d", &iFrameIdc);
		fscanf(m_pIdcFile, "%d", &iTimeStamp);
		fscanf(m_pIdcFile, "%d", &iSizeInBytes);
		fscanf(m_pIdcFile, "%d", &iSizeInBits);
		assert( (iSizeInBits>>3) == iSizeInBytes);
#ifdef WITHONLINEBS
		//if (iFrameIdc) 
        {
            if ( m_pAuLenList[iIdx] != iSizeInBytes ) 
            {
                bNeedSizeReset =  true;
                m_pAuLenList[iIdx] = iSizeInBytes ;
            }
        }
#else		
        if (iFrameIdc)  assert( m_pAuLenList[iIdx] == iSizeInBytes ); 
#endif
		m_pFrameIdcList[iIdx] = iFrameIdc;
		m_pTimestampList[iIdx++] = iTimeStamp;
	}

#ifdef WITHONLINEBS
    if (bNeedSizeReset)
    {
        int32_t i=0;
        m_pAuLenListLength = 0;
        //reset
        for (i=0;i<iIdx;i++)
        {
            AddOneAU(m_pAuLenList[i]);
        }
    }
#endif
    assert( iIdx == m_pAuLenListLength );


	return true;
}






int CInputBitstream::GetAUTotalNum()
{
	return m_pAuLenListLength;
}

int CInputBitstream::GetCurrentAUTime(const int iIdx)
{
	return m_pTimestampList[iIdx];
}

int CInputBitstream::GetCurrentAUSize(const int iIdx)
{
	return m_pAuLenList[iIdx];
}

int CInputBitstream::GetCurrentAUPriority(const int iIdx)
{
	switch (m_pFrameIdcList[iIdx])
	{
	case 19:
		return 11;
	case 18://T2
		return 6;
	case 17://T1
		return 3;
	case 16://T0
		return 2;
	case 8://LTR
		return 1;
	case 0: //IDR
		return 0;
	default: 
		return 15;
	}
}




int CInputBitstream::GetCurrentAUPacketNum(const int iIdx, const int iPacketLength)
{
    const int iPacketNum =  ((m_pAuLenList[iIdx]/iPacketLength) + ((m_pAuLenList[iIdx]%iPacketLength)?1:0) );
	if (iPacketNum>=MAX_PACKET_NUM) return -1;//TODO

	int i=0, iPacketLengthOffset = 0;
	//#0~#(n-1) packets
	while(i<(iPacketNum-1))
	{
		m_pPacketPointerList[i]=m_pAUPointerList[iIdx]+iPacketLengthOffset;
		m_pPacketLenList[i]=iPacketLength;
		iPacketLengthOffset += iPacketLength; 
		i++;
	}
	//#(n-1) packet
	assert(iPacketLengthOffset == iPacketLength*(iPacketNum-1));
	m_pPacketPointerList[i] = m_pAUPointerList[iIdx] + iPacketLengthOffset;
	m_pPacketLenList[i]      = m_pAuLenList[iIdx]      - iPacketLengthOffset;

	return iPacketNum;
}

BS_Element* CInputBitstream::GetCurrentPacketPointer(const int iPacketIdx)
{
	return m_pPacketPointerList[iPacketIdx];
}

int CInputBitstream::GetCurrentPacketLen(const int iPacketIdx)
{
	return m_pPacketLenList[iPacketIdx];
}


FILE* CInputBitstream::GetOutputFile()
{
	return m_pOutputBitstream;
}

//----
//privates
int CInputBitstream::AddOneAU(const int iAuLen)
{
	assert(iAuLen>4);

	if (m_pAuLenListLength)
		m_pAUPointerList[m_pAuLenListLength]=m_pAUPointerList[m_pAuLenListLength-1]+
		                                                          m_pAuLenList[m_pAuLenListLength-1];
	else//first AU
		m_pAUPointerList[m_pAuLenListLength]=m_pInputBsBuffer;

	m_pAuLenList[m_pAuLenListLength++] = iAuLen;
	return 0;
}
int CInputBitstream::FindNextNalHeader(const int iCurPos)
{
	int32_t header = 0;
	int32_t iOffsetPos = iCurPos+4;
	BS_Element* pInputBufferPointer = m_pInputBsBuffer+iOffsetPos;

	while ( iOffsetPos < (m_iBitstreamLength-5) )
	{	
		if (IsNalHeader(iOffsetPos))
		{
			GetNalType(iOffsetPos);	
            //GetFirstMB(iOffsetPos);	
			return (iOffsetPos-iCurPos);		
		}
		++pInputBufferPointer;
		++iOffsetPos;
	}
	return (m_iBitstreamLength - iCurPos);//last AU
}

bool CInputBitstream::IsNalHeader(const int iCurPos)
{
	return (LD32(m_pInputBsBuffer+iCurPos) == start_code_prefix_annexeb);	
	// optimized by Steven due cost 83.89% over extracting provided by Intel(R) VTUNE, 3/30/2011

//	#define is_nal_header(bs) \
//    ((0 == bs[0]) && (0 == bs[1]) && (0 == bs[2]) && (1 == bs[3]))
}
bool CInputBitstream::GetNalType(const int iCurPos)
{	
	BS_Element* pInputBufferPointer = m_pInputBsBuffer+iCurPos;
	//get current NAL type
	m_iTheNalType = *(pInputBufferPointer+NAL_HEADER_IDENTIFIER) & 0x1F;

	m_bIsTheNalVcl = m_bIsTheNalIdr = m_bIsTheNalSps = false;
	switch ( m_iTheNalType )
	{
	case NAL_UNIT_CODED_SLICE_EXT: 
		{
			m_bIsTheNalIdr= static_cast<bool>( *(pInputBufferPointer+NAL_HEADER_LENGTH+1) & 0x40 );    			
			m_bIsTheNalVcl = true;
		}
		//no break to continue checking
	case NAL_UNIT_CODED_SLICE: 
		{
			m_bIsTheNalVcl = true;
			return true;
		}
	case NAL_UNIT_CODED_SLICE_IDR: 
		{
			m_bIsTheNalIdr =  true;
			m_bIsTheNalVcl = true;
			return true;
		}
	case NAL_UNIT_SPS:
		m_bIsTheNalSps = true;
	case NAL_UNIT_PPS: 
	case NAL_UNIT_SUBSET_SPS:
    case NAL_UNIT_SEI:
        m_bIsTheNalSEI = true;
	default:
		m_bIsTheNalVcl = false;
		return false;
	}
}







/*
bool CInputBitstream::GetFirstMB(const int iCurPos)
{	
	BS_Element* pInputBufferPointer = m_pInputBsBuffer+iCurPos;
	//get current NAL type
	m_iTheNalType = *(pInputBufferPointer+NAL_HEADER_IDENTIFIER) & 0x1F;

	m_bIsTheNalVcl = m_bIsTheNalIdr = m_bIsTheNalSps = false;
	switch ( m_iTheNalType )
	{
	case NAL_UNIT_CODED_SLICE_EXT: 
		{
			m_bIsTheNalIdr= static_cast<bool>( *(pInputBufferPointer+NAL_HEADER_LENGTH+1) & 0x40 );    			
			m_bIsTheNalVcl = true;
		}
		//no break to continue checking
	case NAL_UNIT_CODED_SLICE: 
		{
			m_bIsTheNalVcl = true;
			return true;
		}
	case NAL_UNIT_CODED_SLICE_IDR: 
		{
			m_bIsTheNalIdr =  true;
			m_bIsTheNalVcl = true;
			return true;
		}
	case NAL_UNIT_SPS:
		m_bIsTheNalSps = true;
	case NAL_UNIT_PPS: 
	case NAL_UNIT_SUBSET_SPS:
    case NAL_UNIT_SEI:
        m_bIsTheNalSEI = true;
	default:
		m_bIsTheNalVcl = false;
		return false;
	}
}
*/




