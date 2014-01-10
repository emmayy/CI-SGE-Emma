
#ifndef INPUT_BITSTREAM_H__
#define INPUT_BITSTREAM_H__

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <assert.h>
#include "WelsCommonType.h"
#include "WelsMemory.h"

using namespace std;

#define NAL_HEADER_IDENTIFIER      (4) //00 00 00 01
#define NAL_HEADER_LENGTH           (NAL_HEADER_IDENTIFIER+1) //00 00 00 01 + 1byte = 5bytes
#define MAX_PACKET_NUM                (1000)
/*
 *	NAL Unit Type (5 Bits)
 */
typedef uint8_t		Nal_Unit_Type;
enum{
	NAL_UNIT_UNSPEC_0			= 0,
	NAL_UNIT_CODED_SLICE		= 1,
	NAL_UNIT_CODED_SLICE_DPA	= 2,
	NAL_UNIT_CODED_SLICE_DPB	= 3,
	NAL_UNIT_CODED_SLICE_DPC	= 4,
	NAL_UNIT_CODED_SLICE_IDR	= 5,
	NAL_UNIT_SEI				= 6,
	NAL_UNIT_SPS				= 7,
	NAL_UNIT_PPS				= 8,
	NAL_UNIT_AU_DELIMITER		= 9,
	NAL_UNIT_END_OF_SEQ			= 10,
	NAL_UNIT_END_OF_STR			= 11,
	NAL_UNIT_FILLER_DATA		= 12,
	NAL_UNIT_SPS_EXT			= 13,
	NAL_UNIT_PREFIX				= 14,
	NAL_UNIT_SUBSET_SPS			= 15,
	NAL_UNIT_RESV_16			= 16,
	NAL_UNIT_RESV_17			= 17,
	NAL_UNIT_RESV_18			= 18,
	NAL_UNIT_AUX_CODED_SLICE	= 19,
	NAL_UNIT_CODED_SLICE_EXT	= 20,
	NAL_UNIT_RESV_21			= 21,
	NAL_UNIT_RESV_22			= 22,
	NAL_UNIT_RESV_23			= 23,
	NAL_UNIT_UNSPEC_24			= 24,
	NAL_UNIT_UNSPEC_25			= 25,
	NAL_UNIT_UNSPEC_26			= 26,
	NAL_UNIT_UNSPEC_27			= 27,
	NAL_UNIT_UNSPEC_28			= 28,
	NAL_UNIT_UNSPEC_29			= 29,
	NAL_UNIT_UNSPEC_30			= 30,
	NAL_UNIT_UNSPEC_31			= 31
};


#ifndef BS_Element
#define BS_Element uint8_t 
#endif

#if !defined(WORDS_BIGENDIAN)
static const uint32_t start_code_prefix_annexeb			= 0x01000000;
#else
static const uint32_t start_code_prefix_annexeb			= 0x00000001;
#endif//WORDS_BIGENDIAN

#define LD32(a) (*((uint32_t*)(a)))

class CInputBitstream
{
public:
	CInputBitstream();
	CInputBitstream( const char *sBitstreamName, const char *sIdcFileName, const char *sOutputBitstreamName );
	CInputBitstream( const string& sBitstreamName, const string& sIdcFileName, const string& sOutputBitstreamName );
	virtual ~CInputBitstream();
	
	bool ReadBitstreamIntoList();
	
	int GetAUTotalNum();

	int GetCurrentAUTime(const int iIdx);
	int GetCurrentAUSize(const int iIdx);
	int GetCurrentAUPriority(const int iIdx);
	int GetCurrentAUPacketNum(const int iIdx, const int iPacketLength);

	BS_Element* GetCurrentPacketPointer(const int iPacketIdx);
	int GetCurrentPacketLen(const int iPacketIdx);

	//void SaveOutputBitstream(const int iIdx);
	//void OutputBitstreamToFile(const char *sOutputBitstreamName);
	FILE* GetOutputFile();
private:
	int FindNextNalHeader(const int iCurPos);
	bool IsNalHeader(const int iCurPos);
	bool GetNalType(const int iCurPos);
	int AddOneAU(const int iAuLen);

private:
	FILE			*m_pBitstream;
	FILE			*m_pIdcFile;
	FILE			*m_pOutputBitstream;

	int            m_iBitstreamLength;
	
	BS_Element* m_pInputBsBuffer;//actual memory

	BS_Element** m_pAUPointerList;//pointer	
	int*              m_pAuLenList;	
	int                m_pAuLenListLength;
	int*              m_pFrameIdcList;
	int*              m_pTimestampList;

	int				   m_iTheNalType;	
	bool			   m_bIsTheNalVcl;
	bool             m_bIsTheNalIdr;
	bool             m_bIsTheNalSps;
    bool             m_bIsTheNalSEI;

	BS_Element*              m_pPacketPointerList[MAX_PACKET_NUM];	
	int              m_pPacketLenList[MAX_PACKET_NUM];	
};

#endif	// INPUT_BITSTREAM_H__