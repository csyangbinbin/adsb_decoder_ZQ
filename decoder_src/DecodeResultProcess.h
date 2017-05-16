#ifndef __ADSB_DECODE_RESULT_PROCESS_INCLUDE__
#define __ADSB_DECODE_RESULT_PROCESS_INCLUDE__

#include "ADS_B_Decode.h"
#include "adsb_core_decoder.h"
#include "ADSB_FPGA_DecoderBase.h"
#include "../adsb_decoder_api.h"

class ADSB_ResultProcess
{

public:
	ADSB_ResultProcess(adsb_core_decoder* ads_decoder) ;
	virtual ~ADSB_ResultProcess() ;

public:
	bool Init() ;
public:
	adsb_decode_result_t ProcessResult(  DWORD DecodeResult ,  BYTE ADS_B_MSG[14] , 
		const ADS_Time & timestamp , extra_info* ei) ;

	adsb_decode_result_t ProcessDevInfo(extra_info* ei) ;


private:
	adsb_decode_result_t ProcessID_Pack( const AircraftInfo* info ,BYTE ADS_B_MSG[14] , 
		const ADS_Time & timestamp ,  extra_info* ei)  ;

	adsb_decode_result_t ProcessLocation_Pakc( const AircraftInfo* info ,BYTE ADS_B_MSG[14] ,
		const ADS_Time & timestamp ,  extra_info* ei) ;

	adsb_decode_result_t ProcessSPLocation_Pack( const AircraftInfo* info ,BYTE ADS_B_MSG[14] ,
		const ADS_Time & timestamp ,  extra_info* ei) ;

	adsb_decode_result_t ProcessVel_Pack( const AircraftInfo* info ,BYTE ADS_B_MSG[14] , 
		const ADS_Time & timestamp,  extra_info* ei) ;


private:
	adsb_core_decoder* m_deHandle ;
}  ;


#endif
