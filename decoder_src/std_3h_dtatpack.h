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
	ADS-B���ݱ���
	*/
	struct ads_b_data_pack_t
	{
		char			frame_head[3] ; //֡ͷ
		byte			version;		//�汾
		byte			message_type ;	//��Ϣ����
		word			valid_flag;		//��Ч���
		word			station_id;		//��վID
		byte			receiver_type;	//���ջ�����
		byte			channel_info;	//ͨ����Ϣ
		byte			time_tick[5];	//ʱ�����
		sbyte			SNR;			//�����
		dword			spectrum_info;	//Ƶ����Ϣ
		byte			msg_head_match;	//��ͷ��ض�
		byte			msg_invalid_bit_count;	//�����Ŷȱ��ظ���
		byte			message[14];	//ADS-B����
		byte			message_valid_stat[14];	//���Ŷ�
		byte			reserve[9];		//����
		word			crc;			//CRCУ��
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
	������Ϣ����
	*/
	struct common_info_pack_t
	{

		char			frame_head[3] ; //֡ͷ
		byte			version;		//�汾
		byte			message_type ;	//��Ϣ����
		word			valid_flag;		//��Ч���
		word			station_id;		//��վID
		std3h_time_t	time;			//������ʱ����
		byte			gps_info;		//GPS��Ϣ
		byte			gps_location[10];	//ƽ̨��γ��
		byte			time_modify[3];	//ʱ������ֵ
		word			msg_count_pre_sec ;//ÿ����Ϣ����
		byte			dynamic_key[14];	//��̬��Կ
		byte			battery_info[6] ;	//��ص�����Ϣ
		byte			temperature[1] ;		//�¶���Ϣ
		byte			board_channel_state ;   //�����ͨ����Ϣ
		byte			hd_sf_version ;			//Ӳ������汾
		byte			hd_er_version ;			//Ӳ����·���
		byte			reserve[5];		//����
		word			crc;			//CRCУ��	
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
	int64_t		get_ps_tick(byte time_tick[5]) ; //��ȡʱ�����Ӧ��psʱ��

private:
	int max_err_count  ;				//�ɴ��������������
};

#endif
