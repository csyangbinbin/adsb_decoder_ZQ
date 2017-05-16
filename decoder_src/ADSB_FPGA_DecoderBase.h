#ifndef __ADSB_FPGA_DECODER_INTERFACE_INCLUDE__
#define __ADSB_FPGA_DECODER_INTERFACE_INCLUDE__
#include "ADS_B_Decode.h"
#include "fpga_board_info.h"

#define FPGA_TIME_MSG  1
#define FPGA_DATA_MSG  2
#define FPGA_ACARS_MSG 3
#define FPGA_ERROR_DATA  0

typedef char				sbyte ;
typedef unsigned char		byte ;
typedef short				sword ;
typedef unsigned short		word ;
typedef int					sdword;
typedef unsigned int		dword ;



#define  DYNAMIC_KEY_LEN	(14)

//ADS-B解码过程使用的通用信息
struct ads_b_common_info
{
	bool		time_valid  ; //时间信息是否有效
	byte		gps_satellite_count ; //GPS搜星个数
	bool		modify_above ;	//时间校正符号
	unsigned int	time_modify ;	//时间修正值
	byte	year ;	
	byte	month;
	byte	day ;
	byte	hour ;
	byte	minute;
	byte	second ;
	word	msg_count_pre_sec ;//每秒消息数量
	byte	dynamic_key[DYNAMIC_KEY_LEN];	//动态密钥	
	bool	dynamic_key_valid ; //动态密钥是否有效
} ; 

struct extra_info
{
	byte snr ; 
	byte fp[4] ;
	bool valid ; 
	bool gps_time_valid;
	fpga_board_info	hd_info ; 
	unsigned short station_id; 
} ; 


class ADSB_FPGA_DecoderInterface
{
public:
	ADSB_FPGA_DecoderInterface()
	{
	}

	virtual  int Decode(  unsigned char* FPGA_Data  , int len , BYTE*  ADS_Data , 
		ADS_Time& EntryTime , extra_info* ei = NULL) = 0  ;


	ads_b_common_info&	get_common_info() { return common_info ; } 
	virtual bool init() {return true ; }
protected:
	ads_b_common_info common_info ;		//通用解码信息
} ;



#endif
