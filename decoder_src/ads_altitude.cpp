
#include "ads_altitude.h"


unsigned short  gray2bin(unsigned short gray)
{
	unsigned short i = gray >> 1 ; 

	while (i != 0)
	{
		gray ^= i ;
		i >>= 1 ; 
	}
	return gray ; 
}

unsigned short  encode_alt_modes(int alt, int bit13)
{
	bool mbit = false ;
	bool qbit = true ;
	unsigned short encalt = (int(alt) + 1000) / 25 ; 

	unsigned short tmp1 = 0 ;
	unsigned short tmp2 = 0;
	if (bit13) 
	{
		 tmp1 = (encalt & 0xfe0) << 2 ;
		tmp2 = (encalt & 0x010) << 1 ; 
	}
	else
	{
		  tmp1 = (encalt & 0xff8) << 1 ; 
		 tmp2 = 0 ; 
	}

	return (encalt & 0x0F) | tmp1 | tmp2 | (mbit << 6) | (qbit << 4) ; 
}




int  decode_alt(unsigned short alt, int bit13)
{
	int mbit = alt & 0x0040 ; 
	int qbit = alt & 0x0010 ; 
	int decoded_alt = 0 ;

	/*
	#nobody uses metric altitude: AFAIK, it's an orphaned part of
	#the spec. haven't seen it in three years. as a result, replies
	#with mbit set can be considered spurious, and so we discard them here.

	#bits 20-25, 27-31 encode alt in meters
	#remember that bits are LSB (bit 20 is MSB)
	#meters_alt = 0
	#for (shift, bit) in enumerate(range(31,26,-1)+range(25,19,-1)):
	#	meters_alt += ((alt & (1<<bit)) != 0) << shift
	#decoded_alt = meters_alt / 0.3048
	*/
	if (mbit&& bit13)
		return -1;



	if (qbit) //a mode S-style reply
	{
		unsigned int tmp1   =  0;
		unsigned short tmp2 =  0;
		//bit13 is false for BDS0,5 ADS-B squitters, and is true otherwise
		if (bit13)
		{
			/*
			#in this representation, the altitude bits are as follows:
			# 12 11 10 9 8 7 (6) 5 (4) 3 2 1 0
			# so bits 6 and 4 are the M and Q bits, respectively.
			*/
			 tmp1 = (alt & 0x3F80) >> 2;
			 tmp2 = (alt & 0x0020) >> 1;
		}
		else
		{
			tmp1 = (alt & 0x1FE0) >> 1;
			tmp2 = 0;
		}
		decoded_alt = ((alt & 0x0F) | tmp1 | tmp2) * 25 - 1000 ; 
	}
	else //#a mode C-style reply
	{ 	
	
		/*#okay, the order they come in is:
#C1 A1 C2 A2 C4 A4 X B1 D1 B2 D2 B4 D4
#the order we want them in is:
#D2 D4 A1 A2 A4 B1 B2 B4
#so we'll reassemble into a Gray-coded representation
		*/

		unsigned short alt = 0 ;

	  	if (!bit13) 
			   alt = (alt & 0x003F) | (alt & 0x0FC0 << 1) ; 

			unsigned short C1 = 0x1000;
			unsigned short A1 = 0x0800;
			unsigned short C2 = 0x0400;
			unsigned short A2 = 0x0200;	//this represents the order in which the bits come
			unsigned short C4 = 0x0100;
			unsigned short A4 = 0x0080;
			unsigned short B1 = 0x0020;
			unsigned short D1 = 0x0010;
			unsigned short B2 = 0x0008;
			unsigned short D2 = 0x0004;
			unsigned short B4 = 0x0002;
			unsigned short D4 = 0x0001;

		unsigned short bigpart =  ((alt & B4) >> 1) \
		+ ((alt & B2) >> 2) \
		+ ((alt & B1) >> 3) \
		+ ((alt & A4) >> 4) \
		+ ((alt & A2) >> 5) \
		+ ((alt & A1) >> 6) \
		+ ((alt & D4) << 6) \
		+ ((alt & D2) << 5) ; 

//#bigpart is now the 500-foot-resolution Gray-coded binary part
		decoded_alt = gray2bin(bigpart) ; 
//#real_alt is now the 500-foot-per-tick altitude

		unsigned short     cbits =   ((alt & C4) >> 8) + ((alt & C2) >> 9) + ((alt & C1) >> 10) ; 
	    unsigned short  	cval = gray2bin(cbits);// #turn them into a real number

		if (cval == 7)
	      cval = 5 ; // #not a real gray code after all

		if (decoded_alt % 2)
	      cval = 6 - cval; //#since the code is symmetric this unwraps it to see whether to subtract the C bits or add them

		decoded_alt *= 500  ; //#take care of the A,B,D data
		decoded_alt += cval * 100  ;  //#factor in the C data
		decoded_alt -= 1300  ;//#subtract the offset
	}

		return decoded_alt;
	}




unsigned short get_ads_b_alt_word(const unsigned char ads_msg[14])
{
	unsigned short  alt_word = 0;
	alt_word = (((unsigned short)(ads_msg[6]&0xF0))&0xFF)| ((((unsigned short)ads_msg[5])&0xFF)<<8) ; 
	alt_word = alt_word>>4 ;
	return alt_word ; 
}




	



















