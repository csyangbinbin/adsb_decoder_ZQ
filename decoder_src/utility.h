#ifndef __ADS_B_UTILITY_INCLUDE__
#define __ADS_B_UTILITY_INCLUDE__

#include "ADS_B_Decode.h"
#include <time.h>

#ifndef XBIT
#define XBIT(pb,i) ((bool)( (((BYTE)pb[(i)/8])>>(7-((i)%8)))&0x01 ))
#endif

#ifndef MAKEDWORD_h2l
#define MAKEDWORD_h2l(a,b,c,d)  (DWORD)((((DWORD)a)<<24)|(((DWORD)b)<<16)|(((DWORD)c)<<8)|((DWORD)d))
#endif


DWORD CRC24(BYTE* pData,int len) ;
const char* ICAO2Str(ICAOAddress ICAO) ;
ICAOAddress GetICAO(const BYTE* ads_msg) ;


#ifndef MAKEWORD
#define MAKEWORD(a, b)      ((WORD)(((BYTE)((DWORD)(a) & 0xff)) | ((WORD)((BYTE)((DWORD)(b) & 0xff))) << 8))
#endif


bool FrameDecodeByCPU( BYTE SrcData[14], BYTE ProcessedData[14],
					  int ErrorCount, unsigned char* ErrorIndex) ; 

#endif
