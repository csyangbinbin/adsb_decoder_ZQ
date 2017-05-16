#include <string.h>
#include "std_3h_dtatpack.h"
#include "utility.h"
#include "../debug.h"
#ifndef WIN32
 #include <arpa/inet.h>
#else
#include <WinSock2.h>
#endif



int64_t  STD3H_TICK_UINT[16] =
{
	1,			//1 ps
	10,			//10 ps
	50,			//50 ps
	200,		//200 ps
	1*1000,		//1 ns
	2*1000,		//2 ns
	5*1000,		//5 ns
	10*1000,	//10 ns
	20*1000,	//20 ns
	50*1000,	//50 ns
	100*1000,	//100 ns
	200*1000 ,	//200 ns
	500*1000 ,	//500 ns
	1*1000000,	//1 us
	5*1000000,	//5 us
	10*1000000	//10 us
} ;
int64_t STD3H_HZ_MAP[16] =
{
	1000000000000 ,		//1 ps
	100000000000 ,		//10 ps
	20000000000 ,		//50 ps
	5000000000 ,		//200 ps
	1000000000 ,		//1 ns
	500000000,			//2	ns
	200000000,			//5 ns
	100000000,			//10 ns
	50000000,			//20 ns
	20000000,			//50 ns
	10000000,			//100 ns
	5000000,			//200 ns
	2000000,			//500 ns
	1000000,			//1 us
	200000,				//5 us
	100000				//10 us
} ; 


std3h_fpga_decoder::std3h_fpga_decoder()
:max_err_count(10)
{
	memset(&common_info , 0 ,sizeof(ads_b_common_info));
	common_info.time_valid = false ;
	common_info.dynamic_key_valid = false ;
}


bool std3h_fpga_decoder::init()
{
	return true;
}


bool std3h_fpga_decoder::check_frame_head(unsigned char * FPGA_Data)
{
	//ÿ������֡�Ŀ�ͷΪ 0D0ABE
	if(FPGA_Data[0] == 0x0D &&
		FPGA_Data[1] == 0x0A &&
		FPGA_Data[2] == 0xBE)
		return true;
	else
		return false ;
}

bool std3h_fpga_decoder::check_crc(unsigned char* data , int len)
{
	return true ; 	
}


int std3h_fpga_decoder::Decode( unsigned char * FPGA_Data  , int len , BYTE*  ADS_Data ,  
							   ADS_Time& EntryTime , extra_info* ei)
{
	if(!check_crc(FPGA_Data , len))
	{
		log_print( "CRC error!\r\n") ; 
		return FPGA_ERROR_DATA ;
	}

	if (len != STD3H_DATA_PACK_LEN)
	{
		log_print("len != FPGA_CORRECT_DATA_LEN\r\n") ;
		return FPGA_ERROR_DATA;
	}


	//ȷ��֡ͷ�Ƿ���ȷ 
	if(!check_frame_head(FPGA_Data))
	{
		log_print("frame_head error!\r\n");
		return 	FPGA_ERROR_DATA ; 
	}

	byte version = FPGA_Data[3] ;			//���İ汾��
	byte message_type = FPGA_Data[4] ;	 //������Ϣ����


	if(STD3H_COMMON_MSG_TYPE_BYTE == message_type)	//FPAG���͵Ĺ�����Ϣ����
	{
		decode_common_message(FPGA_Data , ei);
		return FPGA_TIME_MSG ; 
	}
	else if(STD3H_ADS_B_MSG_TYPE_BYTE == message_type)	//FPGA���͵�ADS-B��Ϣ����
	{
		return decode_ads_b_message(FPGA_Data ,ADS_Data , EntryTime , ei ) ;
	}
	else		//���������ͱ���
	{
		log_print( "δ֪����Ϣ����!\r\n");
		return FPGA_ERROR_DATA ; 
	}
}
int std3h_fpga_decoder::decode_hd_board_battery_info(board_battery_info* info ,const std3h_detail::common_info_pack_t* common_pack) 
{
	/*
	typedef struct board_battery_info_tag
	{
	unsigned char state ;		//�����߷ŵ�(1--�ŵ� , 0--���)
	unsigned char capability ;	//��ǰ�����İٷֱ�
	unsigned char voltage ;		//��ѹ(��ȡ�����10)
	unsigned char current ;		//����(��ȡ�����100)
	unsigned short	work_time; //����ŵ�ʱ��
	char battery_temperature ;		//����¶�(����)
	}board_battery_info ; 
	*/
	info->state = (common_pack->battery_info[0]>>7)&0x01 ; 
	info->capability = common_pack->battery_info[0]&0x7F ; 
	info->voltage = common_pack->battery_info[1] ; 
	info->current = common_pack->battery_info[2] ; 
	info->work_time =ntohs( *((unsigned short*)&common_pack->battery_info[3]) ) ; 

	bool is_ba_negative = (common_pack->battery_info[5]>>7)&0x01 ;  //����¶���Ϣ
	unsigned char ba_value = common_pack->battery_info[5]&0x7F;
	info->battery_temperature	 =  is_ba_negative?(-1 * ba_value):ba_value ; 


	log_print("���:%d	%d	%d	%d	%d	%d" ,info->state , info->capability , 
		info->voltage ,info->current  , info->work_time  ,info->battery_temperature);

	return 1 ; 
}


int std3h_fpga_decoder::decode_hd_board_temperature_info(board_temperature_info* info ,const std3h_detail::common_info_pack_t* common_pack) 
{
	/*
	//���հ���¶���Ϣ
	typedef struct board_temperature_info_tag
	{
	char board_temperature	;		//�źŴ�����¶�(����)
	}board_temperature_info ;
	*/
	bool is_bo_negative = (common_pack->temperature[0]>>7)&0x01 ; 
	unsigned char bo_value = common_pack->temperature[0]&0x7F ; 
	info->board_temperature		 =  is_bo_negative?(-1 * bo_value ):bo_value ;

	log_print("���հ��¶�:%d" ,  info->board_temperature );

	return 1 ;
}

int std3h_fpga_decoder::decode_hd_board_gps_info(board_gps_info* info ,const std3h_detail::common_info_pack_t* common_pack) 
{
	/*
	typedef struct board_gps_info_tag
	{
	unsigned char	gps_state;		//GPS״̬��Ϣ
	int	longitude ;					 //����
	int	latitude	;				//γ��
	int 	altitude	;				//�߶�
	}board_gps_info;
	*/

	info->gps_state = common_pack->gps_info ; 

	unsigned char LEW	= (common_pack->gps_location[0]>>7)&0x01 ; 
	unsigned char LNS	=(common_pack->gps_location[0]>>6)&0x01 ;
	unsigned char LALT	= (common_pack->gps_location[0]>>5)&0x01 ; 

	typedef union{
		unsigned char uc[4];
		unsigned int ui ; 
	}uuint ; 

	uuint uulon_du ;
	uulon_du.uc[3] = 0 ;
	uulon_du.uc[2] = common_pack->gps_location[0]&0x03;
	uulon_du.uc[1] = common_pack->gps_location[1] ;
	uulon_du.uc[0] = common_pack->gps_location[2]&0xF8 ;

	unsigned int lon_du =  uulon_du.ui >>3 ;  //���ȵĶȷֲ���

	uuint uulon_x ;
	uulon_x.uc[3] = 0 ; 
	uulon_x.uc[2] = common_pack->gps_location[2]&0x07; 
	uulon_x.uc[1] =common_pack->gps_location[3] ; 
	uulon_x.uc[0] =common_pack->gps_location[4]&0xE0 ; 

	unsigned int lon_x = uulon_x.ui>>5 ;	//���ȵ�С������


	uuint uulat_du ; 
	uulat_du.uc[3] = 0 ; 
	uulat_du.uc[2] = common_pack->gps_location[4]&0x1F ; 
	uulat_du.uc[1] = common_pack->gps_location[5] ; 
	uulat_du.uc[0] = common_pack->gps_location[6] &0xC0;

	unsigned int lat_du = uulat_du.ui>>6 ;  //γ�ȵĶȷֲ���

	uuint uulat_x ;
	uulat_x.uc[3] =0;
	uulat_x.uc[2] =0;
	uulat_x.uc[1] =common_pack->gps_location[6]&0x3F;
	uulat_x.uc[0] =common_pack->gps_location[7];
	unsigned int lat_x = uulat_x.ui ; 

	unsigned short alt_tmp = *((unsigned short*)&common_pack->gps_location[8]) ; 
	unsigned short alt_value = ntohs(alt_tmp) ; //�߶���Ϣ


	unsigned int lon_st = lon_du/100 ;
	double		gps_lon = lon_st + (lon_du - lon_st*100+ lon_x/10000.0)/60.0 ;			//GPS����

	gps_lon = LEW? gps_lon : (-1 * gps_lon) ; 


	unsigned int lat_st = lat_du/100;
	double		gps_lat = lat_st + (lat_du-lat_st*100 + lat_x/10000.0)/60.0	 ;		//GPSγ��

	gps_lat = LNS? gps_lat: (-1 * gps_lat) ;

	double gps_alt = alt_value ; 

	gps_alt	 = LALT? gps_alt: (-1*gps_alt) ;					//GPS�߶���Ϣ


	info->longitude = /*htonl*/(static_cast<u_long>(gps_lon*1000000 )) ; 
	info->latitude = /*htonl*/(static_cast<u_long>(gps_lat*1000000));
	info->altitude = /*htonl*/(static_cast<u_long>(gps_alt)) ;

	log_print("ƽ̨λ��:%.5f	%.5f	%.1f" ,gps_lon ,gps_lat ,  gps_alt );

	return 1 ; 
}



int std3h_fpga_decoder::decode_common_message(unsigned char * Data  , extra_info* ei )
{
	if(Data == NULL)
		return FPGA_ERROR_DATA ; 


	fpga_board_info& board_info = ei->hd_info ; //FPGA���հ�ͨ����Ϣ
	memset(&board_info , 0 , sizeof	(fpga_board_info)	) ;


	

	std3h_detail::common_info_pack_t* common_pack = (std3h_detail::common_info_pack_t*)(Data) ; 

	ei->station_id  = ntohs(common_pack->station_id); //��վ���

	board_info.valid_flag = ntohs(common_pack->valid_flag) ; 

	word	valid_flag = board_info.valid_flag;//������Ч���(�����ֽ���)

	bool time_valid = ((valid_flag>>13)&0x0001) &&  \
		((valid_flag>>12)&0x0001 ); //��������ʱ�������Ч

	bool dynamic_key_valid		= (valid_flag>>7)&0x0001 ;  //��̬��Կ�Ƿ���Ч
	bool count_pre_sec_valid	= (valid_flag>>8)&0x0001 ; //ÿ����Ϣ�����Ƿ���Ч 
	bool battery_info_valid		= (valid_flag>>6)&0x0001 ; //��ص�����Ϣ�Ƿ���Ч
	bool temperature_valid		= (valid_flag>>5)&0x0001 ; //�¶���Ϣ�Ƿ���Ч
	bool gsp_vel_valid			= (valid_flag>>1)&0x0001 ; //GPS�ٶ���Ϣ�Ƿ���Ч

	bool station_id_valid		= (valid_flag>>14)&0x0001 ; //��վ����Ƿ���Ч
//	if(station_id_valid)
	//{
		//��վ�����Ϣ��Ч
		board_info.station_id = ntohs(common_pack->station_id) ; 
//	}

	if(gsp_vel_valid)
	{
		unsigned int vel_value = (((unsigned short)Data[55])<<8)|((unsigned short)(Data[56]));
		vel_value >>= 2;
		board_info.vel_info.vel_value  = (vel_value) ;  //���ʴ�С

		unsigned int vel_heading = (((unsigned short)Data[56])<<16)|(((unsigned short)Data[57])<<8)|((unsigned short)(Data[58]));
		vel_heading &= 0x0003FFC0 ; 
		vel_heading >>=6 ; 
		board_info.vel_info.vel_heading = (vel_heading) ; 

	}
	else
	{
		board_info.vel_info.vel_heading = 0xFFFFFFFF ; 
		board_info.vel_info.vel_value	= 0xFFFFFFFF ; 
	}

	bool gps_info_valid = ((valid_flag>>10)&0x0001)&&((valid_flag>>11)&0x0001) ; 
	if(gps_info_valid)
	{
		decode_hd_board_gps_info(&board_info.gps_info ,common_pack ) ; 
	}

	if(battery_info_valid)
	{//�����Ϣ��Ч
		decode_hd_board_battery_info(&board_info.battery_info , common_pack) ; 
	}

	if(temperature_valid)
	{
		//�¶���Ϣ��Ч
		decode_hd_board_temperature_info(&board_info.temperature_info , common_pack) ;
	}

	//ͳ��ͨ����Ϣ�� byte52��
	board_info.c_state.id = common_pack->board_channel_state >>4 ; 
	board_info.c_state.state = common_pack->board_channel_state&0x0F ;
	board_info.hd_sf_version = common_pack->hd_sf_version ;
	board_info.hd_version = common_pack->hd_er_version ; 



	if(time_valid)				//ʱ����Ϣ
	{
		common_info.time_valid = true ;

		board_info.current_time.year =common_info.year = common_pack->time.year ; 
		board_info.current_time.month =common_info.month = common_pack->time.month ; 
		board_info.current_time.day =common_info.day = common_pack->time.day ;
		board_info.current_time.hour =common_info.hour = common_pack->time.hour ;
		board_info.current_time.minute =common_info.minute  = common_pack->time.minute ;
		board_info.current_time.second =common_info.second  = common_pack->time.second ; 


		//Ӳ��ʱ��ƫ�컹��ƫ��
		common_info.modify_above = (common_pack->time_modify[0])>>7 &0x01 ;

		typedef union
		{
			byte	s[4];
			dword	d;
		}ds_dword ; 

		ds_dword dsd ;
		dsd.s[3] = 00 ;
		dsd.s[2] = (common_pack->time_modify[0]) & 0x7F ;
		dsd.s[1] = common_pack->time_modify[1] ;
		dsd.s[0] = common_pack->time_modify[2] ;

		common_info.time_modify = dsd.d ;

	}
	else
	{
		common_info.time_valid = false ; 
		common_info.time_modify = 0 ; 
	}


	if(dynamic_key_valid)		//��̬��Կ��Ϣ
	{
		//��̬��Կ����
		common_info.dynamic_key_valid = true ; 

		/*
		��̬��Կ�����㷨:
		�����ֽ���ǰ���14�ֽڷֱ���������㣬����14�ֽڵĶ�̬��Կ
		*/


		byte	tmp_key[DYNAMIC_KEY_LEN] ;
		memcpy(tmp_key , Data ,DYNAMIC_KEY_LEN);
		byte sec_key = Data[DYNAMIC_KEY_LEN] ; 
		for(int i=0 ;i<DYNAMIC_KEY_LEN ; i++)
		{
			tmp_key[i] = tmp_key[i] ^ sec_key ; 
		}

		memcpy(common_info.dynamic_key , tmp_key ,DYNAMIC_KEY_LEN );

	}
	else
	{
		//��̬��Կ������
		common_info.dynamic_key_valid = false  ; 
		memset(common_info.dynamic_key , 0 , DYNAMIC_KEY_LEN) ;
	}


	if(count_pre_sec_valid)	//ÿ����Ϣ������Ϣ
	{
		common_info.msg_count_pre_sec =  ntohs(common_pack->msg_count_pre_sec) ; 
	}
	else
	{
		common_info.msg_count_pre_sec =  0 ; 
	}

	return FPGA_TIME_MSG ; 
}


int std3h_fpga_decoder::decode_ads_b_message(unsigned char * Data  ,BYTE*  ADS_Data , 
											 ADS_Time& EntryTime ,  extra_info* ei) 
{
#define ADSB_DF17_SZIE	14 

	bool CorrectOk = true;
	BYTE ProcessedData[ADSB_DF17_SZIE];
	memset(ProcessedData , 0 , ADSB_DF17_SZIE);

	if( (Data == NULL) || (ADS_Data == NULL) )
		return FPGA_ERROR_DATA ; 

	std3h_detail::ads_b_data_pack_t* data_pack = (std3h_detail::ads_b_data_pack_t*)(Data) ; 
	ei->station_id = ntohs(data_pack->station_id); //��վ���

	//���㱨�Ĵ�����Ԫ����
	const byte* ErrorMark =  data_pack->message_valid_stat; 
	unsigned char  intErrorMark[112];
	int error_val = 0;

	for (int i=0; i<112; i++)
	{
		if (!(XBIT(ErrorMark, i)))
		{
			intErrorMark[error_val++] = i;
		}
	}

	word	valid_flag = ntohs(data_pack->valid_flag);//������Ч���
	bool DZXDDY_valid = (valid_flag>>4)&0x0001 ;  //�����Ŷȵ�Ԫ�Ƿ���Ч
	bool BWXX_valid =(valid_flag>>5)&0x0001 ; //�Ƿ��Ѿ���������


	//�����̬��Կ���ã���ԭ���ܵ�ADS-B��14�ֽ�����
	byte in_msg_adsb_data[14] ; 
	if(common_info.dynamic_key_valid )
	{
		for(int i=0;i<14;i++)
			in_msg_adsb_data[i] = (data_pack->message[i]) ^ (common_info.dynamic_key[i]) ; 

	}
	else
	{
		//û�ж�̬��Կ
		memcpy(in_msg_adsb_data , data_pack->message , 14);
	}


	bool src_data_soft_modifyed = false ; //Դ�����Ƿ����������

	if(BWXX_valid && !DZXDDY_valid)
	{
		//�ź��д��󣬵���Ӳ���Ѿ�����
		CorrectOk = true ;
	}
	else if( !BWXX_valid && DZXDDY_valid)
	{
		//�ź��д��󣬵���Ӳ��û�о���,Ŀǰ�����о���
	//	printf("�ź��д�Ӳ��δ���о���\rn");
		CorrectOk = false ;
	}
	else
	{
		//������״��
		CorrectOk = false ;
	}



	if(CorrectOk)
	{
		if(common_info.time_valid) 
		{ 
			if(ei)
				ei->gps_time_valid = true ; //Ӳ��ʱ����Ϣ����

			//���Ӳ��ʱ����ã���ʹ��Ӳ��ʱ���
			int64_t			msg_tick_ps =get_ps_tick(data_pack->time_tick)	;//�������ĵ�ʱ�����ֵ(ps)
			unsigned int	time_modify  = common_info.time_modify ;		//ʱ������ֵ
			byte unit_flag = (data_pack->time_tick[0]>>4)&0x0F ;			//ʱ�䵥λ���
			double modify_factor = static_cast<double>(time_modify) / STD3H_HZ_MAP[unit_flag] ; //ʱ����������


			int64_t correct_time_tick_ps = 0 ; 

			if(common_info.modify_above)
			{
				//Ӳ��ʱ��ƫ��
				correct_time_tick_ps = static_cast<int64_t>(msg_tick_ps * (1.0-modify_factor)) ; 
			}
			else
			{
				//Ӳ��ʱ��ƫ��
				correct_time_tick_ps = static_cast<int64_t>(msg_tick_ps * (1.0+modify_factor)); 
			}

			EntryTime.t.tm_year =	static_cast<int>(common_info.year) +2000  ;
			EntryTime.t.tm_mon  =	common_info.month  ;/* months since January - [0,11] */
			EntryTime.t.tm_mday =	common_info.day ;  /* day of the month - [1,31] */
			EntryTime.t.tm_hour =	common_info.hour ;
			EntryTime.t.tm_min	=	common_info.minute ;
			EntryTime.t.tm_sec	=	common_info.second ;

			EntryTime.tv_msec	=	static_cast<unsigned long>(correct_time_tick_ps /1000000000);
			EntryTime.tv_wsec	= static_cast<unsigned long>((correct_time_tick_ps - static_cast<int64_t>(EntryTime.tv_msec)* 1000000000) /1000000) ; 
			EntryTime.tv_nsec	= static_cast<unsigned long>((correct_time_tick_ps - static_cast<int64_t>(EntryTime.tv_msec)*1000000000 - static_cast<int64_t>(EntryTime.tv_wsec)*1000000)/1000) ; 

		}
		else
		{
			if(ei)
				ei->gps_time_valid = false ; //Ӳ��ʱ����Ϣ������

			memset((void*)&EntryTime , 0 , sizeof(ADS_Time));

		}

		if(ei)
		{
			memcpy(&(ei->snr) , &Data[16] , 1);
			memcpy(&(ei->fp) ,  &Data[17] , 4 );
			ei->valid = true ; 
		}

		if (src_data_soft_modifyed)
			memcpy(ADS_Data, ProcessedData,	14);
		else
			memcpy(ADS_Data, in_msg_adsb_data, 14);

		return FPGA_DATA_MSG ; 
	}
	else
		return FPGA_ERROR_DATA;
}

int64_t		std3h_fpga_decoder::get_ps_tick(byte time_tick[5])
{
	typedef	union int64union
	{
		byte s[8] ; 
		int64_t	 int_value ;
	}int64union_t;


	int64union_t it ;
	it.s[0] = time_tick[4];
	it.s[1] = time_tick[3];
	it.s[2] = time_tick[2];
	it.s[3] = time_tick[1];
	it.s[4] = time_tick[0]&0x0F;
	it.s[5] = 0;
	it.s[6] = 0;
	it.s[7] = 0 ; 

	int64_t tick = it.int_value ;  //ʱ�����

	int  flag = (time_tick[0]>>4)&0x0F ;

	return tick* STD3H_TICK_UINT[flag] ; 

}

int64_t		std3h_fpga_decoder::get_HZ(byte flag)
{
	return STD3H_HZ_MAP[flag] ; 
}
