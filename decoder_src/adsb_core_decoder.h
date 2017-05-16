/********************************************************************
	created:	2013/07/03
	created:	3:7:2013   9:47
	filename: 	i:\AB3.0\3H_FST\FST\ads_b_decoder\adsb_core_decoder.h
	file path:	i:\AB3.0\3H_FST\FST\ads_b_decoder
	file base:	adsb_core_decoder
	file ext:	h
	author:		ybb
	
	purpose:	ADS-B全局解码器
*********************************************************************/
#ifndef __ADSB_GLOBAL_DECODER_INCLUDE__
#define __ADSB_GLOBAL_DECODER_INCLUDE__
#include "ADS_B_Decode.h"

#define ERR_DECODE_TYPE   0
#define ID_DECODE_TYPE    1
#define SP_DECODE_TYPE    2
#define AP_DECODE_TYPE    3
#define VEL_DECODE_TYPE   4
#define OP_DECODE_TYPE    5
#define OTHER_DECODE_TYPE 6


//超时检测间隔(S)
#define CHECK_TIMEOUT_INTERVAL  (10*60)
//目标超时时间(S)
#define MAX_TIMEOUT_TIME        (5*60)

class adsb_core_decoder
	{

	public:
		adsb_core_decoder() ;
		virtual ~adsb_core_decoder() ;

		bool init(double home_lon , double home_lat ) ;
	public:
		DWORD decode( BYTE AdsMessage[14], ADS_Time Time ) ;
		AircraftInfo* get_aircraft_info(const ICAOAddress& id) ; 

	private:
		void check_time_out( const ADS_Time &Time ) ;

	private:
		DE_HANDLE m_deHandle ; 
		time_t m_last_check_time ;
	} ;
#endif