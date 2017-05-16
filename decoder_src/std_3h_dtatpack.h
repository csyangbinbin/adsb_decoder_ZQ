/********************************************************************
created:	2014/01/14
created:	14:1:2014   14:59
filename: 	e:\project\BaseDecoder\BaseDecoder\decoder\std_3h_dtatpack.h
file path:	e:\project\BaseDecoder\BaseDecoder\decoder
file base:	std_3h_dtatpack
file ext:	h
author:		ybb

purpose:	
*********************************************************************/
#ifndef __STD_3H_DATAPACK_INCLUDE__
#define __STD_3H_DATAPACK_INCLUDE__

#include "ADS_B_Decode.h"
#include "ADSB_FPGA_DecoderBase.h"
#include <time.h>
#include "fpga_board_info.h"


#ifndef WIN32
typedef long int int64_t; 
#endif

namespace std3h_detail
{

#pragma  pack(push ,1)


	/*
	ADS-B数据报文
	*/
	struct ads_b_data_pack_t
	{
		char			frame_head[3] ; //帧头
		byte			version;		//版本
		byte			message_type ;	//消息类型
		word			valid_flag;		//有效标记
		word			station_id;		//基站ID
		byte			receiver_type;	//接收机类型
		byte			channel_info;	//通道信息
		byte			time_tick[5];	//时间计数
		sbyte			SNR;			//信噪比
		dword			spectrum_info;	//频谱信息
		byte			msg_head_match;	//报头相关度
		byte			msg_invalid_bit_count;	//低置信度比特个数
		byte			message[14];	//ADS-B报文
		byte			message_valid_stat[14];	//置信度
		byte			reserve[9];		//保留
		word			crc;			//CRC校验
	} ;


	struct	std3h_time_t
	{
		byte	year ;
		byte	month ;
		byte	day;
		byte	hour ;
		byte	minute;
		byte	second ;
	};

	/*  
	公共信息报文
	*/
	struct common_info_pack_t
	{

		char			frame_head[3] ; //帧头
		byte			version;		//版本
		byte			message_type ;	//消息类型
		word			valid_flag;		//有效标记
		word			station_id;		//基站ID
		std3h_time_t	time;			//年月日时分秒
		byte			gps_info;		//GPS信息
		byte			gps_location[10];	//平台经纬高
		byte			time_modify[3];	//时间修正值
		word			msg_count_pre_sec ;//每秒消息数量
		byte			dynamic_key[14];	//动态密钥
		byte			battery_info[6] ;	//电池电量信息
		byte			temperature[1] ;		//温度信息
		byte			board_channel_state ;   //处理板通道信息
		byte			hd_sf_version ;			//硬件程序版本
		byte			hd_er_version ;			//硬件电路设计
		byte			reserve[5];		//保留
		word			crc;			//CRC校验	
	} ;


#pragma  pack(pop) 

} ; 


#define STD3H_DATA_PACK_LEN		62

#define STD3H_COMMON_MSG_TYPE_BYTE		(0x01)
#define STD3H_ADS_B_MSG_TYPE_BYTE		(0x02)


class std3h_fpga_decoder : public ADSB_FPGA_DecoderInterface
{
public:
	std3h_fpga_decoder() ; 
	int Decode(  unsigned char * FPGA_Data  , int len , BYTE*  ADS_Data , 
		ADS_Time& EntryTime ,extra_info* ei) ;
	ads_b_common_info&	get_common_info() ; 
	bool init() ; 

private:
	bool check_frame_head(unsigned char * FPGA_Data);
	bool check_crc(unsigned char* data , int len);
	int decode_common_message(unsigned char * Data  , extra_info* ei );
	int decode_ads_b_message(unsigned char * Data  ,BYTE*  ADS_Data ,
		ADS_Time& EntryTime , extra_info* ei) ;
	int decode_hd_board_battery_info(board_battery_info* info ,const std3h_detail::common_info_pack_t* common_pack ) ;
	int decode_hd_board_temperature_info(board_temperature_info* info ,const std3h_detail::common_info_pack_t* common_pack) ;
	int decode_hd_board_gps_info(board_gps_info* info ,const std3h_detail::common_info_pack_t* common_pack) ;


	int64_t		get_HZ(byte flag) ;
	int64_t		get_ps_tick(byte time_tick[5]) ; //获取时间戳对应的ps时间

private:
	int max_err_count  ;				//可处理的最大错误数量
};

#endif
