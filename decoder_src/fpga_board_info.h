/********************************************************************
created:	2014/02/11
created:	11:2:2014   8:32
filename: 	e:\project\BaseDecoder\BaseDecoder\decoder\fpga_board_info.h
file path:	e:\project\BaseDecoder\BaseDecoder\decoder
file base:	fpga_board_info
file ext:	h
author:		ybb

purpose:	FPGA公共信息
*********************************************************************/
#ifndef	__FPGA_BOARD_INFO_INCLUDE__
#define __FPGA_BOARD_INFO_INCLUDE__

#include "ADSB_DataProtocol.h"

#pragma pack(push,1)

//接收板的电池信息
typedef struct board_battery_info_tag
{
	unsigned char state ;		//充电或者放电(1--放电 , 0--充电)
	unsigned char capability ;	//当前电量的百分比
	unsigned char voltage ;		//电压(获取后除以10)
	unsigned char current ;		//电流(获取后除以10)
	unsigned short	work_time; //充电或放电时间
	char battery_temperature ;	//电池温度(正负)
}board_battery_info ; 

//接收板的温度信息
typedef struct board_temperature_info_tag
{
	char board_temperature	;		//信号处理板温度(正负)
}board_temperature_info ;


//接收板接收的GPS信息
typedef struct board_gps_info_tag
{
	unsigned char	gps_state;		//GPS状态信息
	int	longitude	;				//经度 (获取后1000000)
	int	latitude	;				//纬度	(获取后1000000)
	int altitude	;				//高度

}board_gps_info;

typedef	struct board_time_info_tag
{
	unsigned char	year ;	
	unsigned char	month;
	unsigned char	day ;
	unsigned char	hour ;
	unsigned char	minute;
	unsigned char	second ;
} board_time_info;


typedef struct channel_state
{
int id  ;	//通道ID
int	state ; //通道状态
} channel_state;

typedef struct gps_vel_info
{
unsigned int vel_heading ; //GPS速度方向(0xFFFFFFFF表示无效) 
unsigned int vel_value ;   //GPS速度大小(0xFFFFFFFF表示无效) 

}gps_vel_info;

//接收板通用信息
typedef struct fpga_board_info_tag
{
	unsigned int			tag		;		//结构体标签(0xFF0000A2)
	unsigned short			valid_flag ;	//信息领域有效标志位
	unsigned short			station_id ;	//基站编号
	board_time_info			current_time;	//当前的时间
	board_gps_info			gps_info	;	//GPS信息	
	board_battery_info		battery_info ;	 //电池信息
	board_temperature_info	temperature_info;//温度信息
	channel_state			c_state ;		 //通道状态
	unsigned char			hd_version ;	 //硬件版本
	unsigned char			hd_sf_version;   //硬件中的版本
	gps_vel_info			vel_info ;		  //GPS速度信息	
}fpga_board_info ; 

#define		FPGA_BOARD_INFO_TAG		(0xFF0000A2)



#pragma  pack(pop)


#endif

