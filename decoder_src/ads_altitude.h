/********************************************************************
	created:	2013/08/02
	created:	2:8:2013   15:34
	filename: 	ads_altitude.h
	file path:	
	file base:	ads_altitude
	file ext:	h
	author:		ybb
	
	purpose:	ADS-B高度数据编解码
*********************************************************************/
#ifndef __ADS_ALTITUDE_DECODE_INCLUDE__
#define __ADS_ALTITUDE_DECODE_INCLUDE__

unsigned short  encode_alt_modes(int alt, int bit13) ; 
unsigned short get_ads_b_alt_word(const unsigned char ads_msg[14]) ; 
int  decode_alt(unsigned short alt, int bit13) ; 

#endif