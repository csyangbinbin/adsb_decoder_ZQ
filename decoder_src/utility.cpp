#include <string.h>
#include  "utility.h"
#include <cstdio>


DWORD CRC24(BYTE* pData,int len)
{
        DWORD CRC24_INIT = 0;
        DWORD CRC24_PLOY  = 0x1FFF409;
        int j=0;
        int i=0;
        DWORD ulCRC = CRC24_INIT;
        for(i=0;i<len;i++)
        {
                ulCRC  = ulCRC^(((DWORD)pData[i])<<16);
                for(j = 0;j<=7;j++)
                {
                        ulCRC  = ulCRC <<1;
                        if((ulCRC&0x1000000)!=0)
                                ulCRC = ulCRC^CRC24_PLOY;
                }
        }
        return ( ulCRC&0xFFFFFF);

}



const char* ICAO2Str(ICAOAddress ICAO)
{
static char icaostr[10] ;
sprintf(icaostr  ,  "%02X%02X%02X" , ICAO.m_1 , ICAO.m_2 , ICAO.m_3)  ;
return icaostr ;
}


ICAOAddress GetICAO(const BYTE* ads_msg)
{
  return ICAOAddress(ads_msg[1] , ads_msg[2] ,ads_msg[3]) ;
}


bool FrameDecodeByCPU( BYTE SrcData[14], BYTE ProcessedData[14],
									 int ErrorCount, unsigned char* ErrorIndex)
{
	unsigned int Loop_N = (1<<ErrorCount) -1 ;
	for (unsigned int Index = 1; Index <= Loop_N; Index++)
	{
		BYTE Tmp[14];
		memcpy(Tmp, SrcData, 14);

		for (int i = 0; i < 32; i++)
		{
			if (Index & (1 << i))
			{
				unsigned char  EI = ErrorIndex[i];
				Tmp[EI >> 3] = Tmp[EI >> 3] ^ (1 << (7 - (EI & 7)));
			}

		} //in_for

		DWORD CRC = CRC24(Tmp, 14);
		if (CRC == 0)
		{
			memcpy(ProcessedData, Tmp, 14);
			return true;
		}

	}//out_for
	return false;
}
