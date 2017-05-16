/********************************************************************
created:	2014/02/11
created:	11:2:2014   8:32
filename: 	e:\project\BaseDecoder\BaseDecoder\decoder\fpga_board_info.h
file path:	e:\project\BaseDecoder\BaseDecoder\decoder
file base:	fpga_board_info
file ext:	h
author:		ybb

purpose:	FPGA������Ϣ
*********************************************************************/
#ifndef	__FPGA_BOARD_INFO_INCLUDE__
#define __FPGA_BOARD_INFO_INCLUDE__

#include "ADSB_DataProtocol.h"

#pragma pack(push,1)

//���հ�ĵ����Ϣ
typedef struct board_battery_info_tag
{
	unsigned char state ;		//�����߷ŵ�(1--�ŵ� , 0--���)
	unsigned char capability ;	//��ǰ�����İٷֱ�
	unsigned char voltage ;		//��ѹ(��ȡ�����10)
	unsigned char current ;		//����(��ȡ�����10)
	unsigned short	work_time; //����ŵ�ʱ��
	char battery_temperature ;	//����¶�(����)
}board_battery_info ; 

//���հ���¶���Ϣ
typedef struct board_temperature_info_tag
{
	char board_temperature	;		//�źŴ�����¶�(����)
}board_temperature_info ;


//���հ���յ�GPS��Ϣ
typedef struct board_gps_info_tag
{
	unsigned char	gps_state;		//GPS״̬��Ϣ
	int	longitude	;				//���� (��ȡ��1000000)
	int	latitude	;				//γ��	(��ȡ��1000000)
	int altitude	;				//�߶�

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
int id  ;	//ͨ��ID
int	state ; //ͨ��״̬
} channel_state;

typedef struct gps_vel_info
{
unsigned int vel_heading ; //GPS�ٶȷ���(0xFFFFFFFF��ʾ��Ч) 
unsigned int vel_value ;   //GPS�ٶȴ�С(0xFFFFFFFF��ʾ��Ч) 

}gps_vel_info;

//���հ�ͨ����Ϣ
typedef struct fpga_board_info_tag
{
	unsigned int			tag		;		//�ṹ���ǩ(0xFF0000A2)
	unsigned short			valid_flag ;	//��Ϣ������Ч��־λ
	unsigned short			station_id ;	//��վ���
	board_time_info			current_time;	//��ǰ��ʱ��
	board_gps_info			gps_info	;	//GPS��Ϣ	
	board_battery_info		battery_info ;	 //�����Ϣ
	board_temperature_info	temperature_info;//�¶���Ϣ
	channel_state			c_state ;		 //ͨ��״̬
	unsigned char			hd_version ;	 //Ӳ���汾
	unsigned char			hd_sf_version;   //Ӳ���еİ汾
	gps_vel_info			vel_info ;		  //GPS�ٶ���Ϣ	
}fpga_board_info ; 

#define		FPGA_BOARD_INFO_TAG		(0xFF0000A2)



#pragma  pack(pop)


#endif

