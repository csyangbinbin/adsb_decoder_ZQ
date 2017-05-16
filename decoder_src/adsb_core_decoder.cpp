#include "adsb_core_decoder.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
adsb_core_decoder::adsb_core_decoder()
:m_deHandle(INVALID_DECODE_HANDLE) ,m_last_check_time(0)
{
}

adsb_core_decoder::~adsb_core_decoder() 
{

}

bool adsb_core_decoder::init(double home_lon , double home_lat)
{
	m_deHandle = InitNewDecoder(home_lon , home_lat);
	return m_deHandle!=INVALID_DECODE_HANDLE ; 
}




DWORD adsb_core_decoder::decode( BYTE AdsMessage[14], ADS_Time Time )
{
	if(AdsMessage == NULL) 
		return ERR_DECODE_TYPE ;

	if(INVALID_DECODE_HANDLE == m_deHandle)
		return ERR_DECODE_TYPE ; 

	BYTE DF_Code = (AdsMessage[0]>>3)&0x1F;

	/*
	处理DF=17与DF=18,CF=001的报文
	*/
	if( (DF_Code!=17)&&(DF_Code!=18) )  return ERR_DECODE_TYPE ;


	BYTE Code_Type = (AdsMessage[4]>>3)&0x1F;

	BOOL Result  =  DecodeADSMessage(m_deHandle , AdsMessage  , Time ) ;

	check_time_out(Time) ;

	if(Result == FALSE ) 
		return ERR_DECODE_TYPE ;
	else
	{
		if(DF_Code == 17)
		{
			if(Code_Type>=1 && Code_Type<=4 )  return    ID_DECODE_TYPE ;
			if(Code_Type>=5 && Code_Type<=8 )  return    SP_DECODE_TYPE ;
			if(Code_Type>=9 && Code_Type<=18 ) return    AP_DECODE_TYPE ;
			if(Code_Type==19 )                 return    VEL_DECODE_TYPE ;
			if(Code_Type==31 )                 return    OP_DECODE_TYPE ;
		}
		else if( (DF_Code == 18) &&( (AdsMessage[0]&0x01)==1) )
		{
			return SP_DECODE_TYPE ; 
		}
	}

	return OTHER_DECODE_TYPE  ;
}

void adsb_core_decoder::check_time_out( const ADS_Time &Time )
{
	time_t this_time = time(NULL) ;
	if(difftime(this_time , m_last_check_time ) > CHECK_TIMEOUT_INTERVAL )
	{
		ADS_Time ref_time ;
		ref_time.t  = Time.t ;

		size_t time_out_size = DeleteTimeOutFromRefTime( m_deHandle , ref_time ,  MAX_TIMEOUT_TIME ) ;

		m_last_check_time = this_time ;
	}
}

AircraftInfo* adsb_core_decoder::get_aircraft_info(const ICAOAddress& id) 
{
	return GetAircraftInfo(m_deHandle, id);
}
