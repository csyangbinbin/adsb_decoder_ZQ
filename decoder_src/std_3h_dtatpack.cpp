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
	//每个数据帧的开头为 0D0ABE
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


	//确认帧头是否正确 
	if(!check_frame_head(FPGA_Data))
	{
		log_print("frame_head error!\r\n");
		return 	FPGA_ERROR_DATA ; 
	}

	byte version = FPGA_Data[3] ;			//报文版本号
	byte message_type = FPGA_Data[4] ;	 //报文消息类型


	if(STD3H_COMMON_MSG_TYPE_BYTE == message_type)	//FPAG发送的公共消息类型
	{
		decode_common_message(FPGA_Data , ei);
		return FPGA_TIME_MSG ; 
	}
	else if(STD3H_ADS_B_MSG_TYPE_BYTE == message_type)	//FPGA发送的ADS-B消息类型
	{
		return decode_ads_b_message(FPGA_Data ,ADS_Data , EntryTime , ei ) ;
	}
	else		//其他的类型报文
	{
		log_print( "未知的消息类型!\r\n");
		return FPGA_ERROR_DATA ; 
	}
}
int std3h_fpga_decoder::decode_hd_board_battery_info(board_battery_info* info ,const std3h_detail::common_info_pack_t* common_pack) 
{
	/*
	typedef struct board_battery_info_tag
	{
	unsigned char state ;		//充电或者放电(1--放电 , 0--充电)
	unsigned char capability ;	//当前电量的百分比
	unsigned char voltage ;		//电压(获取后除以10)
	unsigned char current ;		//电流(获取后除以100)
	unsigned short	work_time; //充电或放电时间
	char battery_temperature ;		//电池温度(正负)
	}board_battery_info ; 
	*/
	info->state = (common_pack->battery_info[0]>>7)&0x01 ; 
	info->capability = common_pack->battery_info[0]&0x7F ; 
	info->voltage = common_pack->battery_info[1] ; 
	info->current = common_pack->battery_info[2] ; 
	info->work_time =ntohs( *((unsigned short*)&common_pack->battery_info[3]) ) ; 

	bool is_ba_negative = (common_pack->battery_info[5]>>7)&0x01 ;  //电池温度信息
	unsigned char ba_value = common_pack->battery_info[5]&0x7F;
	info->battery_temperature	 =  is_ba_negative?(-1 * ba_value):ba_value ; 


	log_print("电池:%d	%d	%d	%d	%d	%d" ,info->state , info->capability , 
		info->voltage ,info->current  , info->work_time  ,info->battery_temperature);

	return 1 ; 
}


int std3h_fpga_decoder::decode_hd_board_temperature_info(board_temperature_info* info ,const std3h_detail::common_info_pack_t* common_pack) 
{
	/*
	//接收板的温度信息
	typedef struct board_temperature_info_tag
	{
	char board_temperature	;		//信号处理板温度(正负)
	}board_temperature_info ;
	*/
	bool is_bo_negative = (common_pack->temperature[0]>>7)&0x01 ; 
	unsigned char bo_value = common_pack->temperature[0]&0x7F ; 
	info->board_temperature		 =  is_bo_negative?(-1 * bo_value ):bo_value ;

	log_print("接收板温度:%d" ,  info->board_temperature );

	return 1 ;
}

int std3h_fpga_decoder::decode_hd_board_gps_info(board_gps_info* info ,const std3h_detail::common_info_pack_t* common_pack) 
{
	/*
	typedef struct board_gps_info_tag
	{
	unsigned char	gps_state;		//GPS状态信息
	int	longitude ;					 //经度
	int	latitude	;				//纬度
	int 	altitude	;				//高度
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

	unsigned int lon_du =  uulon_du.ui >>3 ;  //经度的度分部分

	uuint uulon_x ;
	uulon_x.uc[3] = 0 ; 
	uulon_x.uc[2] = common_pack->gps_location[2]&0x07; 
	uulon_x.uc[1] =common_pack->gps_location[3] ; 
	uulon_x.uc[0] =common_pack->gps_location[4]&0xE0 ; 

	unsigned int lon_x = uulon_x.ui>>5 ;	//经度的小数部分


	uuint uulat_du ; 
	uulat_du.uc[3] = 0 ; 
	uulat_du.uc[2] = common_pack->gps_location[4]&0x1F ; 
	uulat_du.uc[1] = common_pack->gps_location[5] ; 
	uulat_du.uc[0] = common_pack->gps_location[6] &0xC0;

	unsigned int lat_du = uulat_du.ui>>6 ;  //纬度的度分部分

	uuint uulat_x ;
	uulat_x.uc[3] =0;
	uulat_x.uc[2] =0;
	uulat_x.uc[1] =common_pack->gps_location[6]&0x3F;
	uulat_x.uc[0] =common_pack->gps_location[7];
	unsigned int lat_x = uulat_x.ui ; 

	unsigned short alt_tmp = *((unsigned short*)&common_pack->gps_location[8]) ; 
	unsigned short alt_value = ntohs(alt_tmp) ; //高度信息


	unsigned int lon_st = lon_du/100 ;
	double		gps_lon = lon_st + (lon_du - lon_st*100+ lon_x/10000.0)/60.0 ;			//GPS经度

	gps_lon = LEW? gps_lon : (-1 * gps_lon) ; 


	unsigned int lat_st = lat_du/100;
	double		gps_lat = lat_st + (lat_du-lat_st*100 + lat_x/10000.0)/60.0	 ;		//GPS纬度

	gps_lat = LNS? gps_lat: (-1 * gps_lat) ;

	double gps_alt = alt_value ; 

	gps_alt	 = LALT? gps_alt: (-1*gps_alt) ;					//GPS高度信息


	info->longitude = /*htonl*/(static_cast<u_long>(gps_lon*1000000 )) ; 
	info->latitude = /*htonl*/(static_cast<u_long>(gps_lat*1000000));
	info->altitude = /*htonl*/(static_cast<u_long>(gps_alt)) ;

	log_print("平台位置:%.5f	%.5f	%.1f" ,gps_lon ,gps_lat ,  gps_alt );

	return 1 ; 
}



int std3h_fpga_decoder::decode_common_message(unsigned char * Data  , extra_info* ei )
{
	if(Data == NULL)
		return FPGA_ERROR_DATA ; 


	fpga_board_info& board_info = ei->hd_info ; //FPGA接收板通用信息
	memset(&board_info , 0 , sizeof	(fpga_board_info)	) ;


	

	std3h_detail::common_info_pack_t* common_pack = (std3h_detail::common_info_pack_t*)(Data) ; 

	ei->station_id  = ntohs(common_pack->station_id); //基站编号

	board_info.valid_flag = ntohs(common_pack->valid_flag) ; 

	word	valid_flag = board_info.valid_flag;//内容有效标记(主机字节序)

	bool time_valid = ((valid_flag>>13)&0x0001) &&  \
		((valid_flag>>12)&0x0001 ); //年月日与时分秒均有效

	bool dynamic_key_valid		= (valid_flag>>7)&0x0001 ;  //动态密钥是否有效
	bool count_pre_sec_valid	= (valid_flag>>8)&0x0001 ; //每秒消息数量是否有效 
	bool battery_info_valid		= (valid_flag>>6)&0x0001 ; //电池电量信息是否有效
	bool temperature_valid		= (valid_flag>>5)&0x0001 ; //温度信息是否有效
	bool gsp_vel_valid			= (valid_flag>>1)&0x0001 ; //GPS速度信息是否有效

	bool station_id_valid		= (valid_flag>>14)&0x0001 ; //基站编号是否有效
//	if(station_id_valid)
	//{
		//基站编号信息有效
		board_info.station_id = ntohs(common_pack->station_id) ; 
//	}

	if(gsp_vel_valid)
	{
		unsigned int vel_value = (((unsigned short)Data[55])<<8)|((unsigned short)(Data[56]));
		vel_value >>= 2;
		board_info.vel_info.vel_value  = (vel_value) ;  //速率大小

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
	{//电池信息有效
		decode_hd_board_battery_info(&board_info.battery_info , common_pack) ; 
	}

	if(temperature_valid)
	{
		//温度信息有效
		decode_hd_board_temperature_info(&board_info.temperature_info , common_pack) ;
	}

	//统计通道信息（ byte52）
	board_info.c_state.id = common_pack->board_channel_state >>4 ; 
	board_info.c_state.state = common_pack->board_channel_state&0x0F ;
	board_info.hd_sf_version = common_pack->hd_sf_version ;
	board_info.hd_version = common_pack->hd_er_version ; 



	if(time_valid)				//时间信息
	{
		common_info.time_valid = true ;

		board_info.current_time.year =common_info.year = common_pack->time.year ; 
		board_info.current_time.month =common_info.month = common_pack->time.month ; 
		board_info.current_time.day =common_info.day = common_pack->time.day ;
		board_info.current_time.hour =common_info.hour = common_pack->time.hour ;
		board_info.current_time.minute =common_info.minute  = common_pack->time.minute ;
		board_info.current_time.second =common_info.second  = common_pack->time.second ; 


		//硬件时钟偏快还是偏慢
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


	if(dynamic_key_valid)		//动态密钥信息
	{
		//动态密钥可用
		common_info.dynamic_key_valid = true ; 

		/*
		动态密钥生成算法:
		用秒字节与前面的14字节分别作异或运算，生成14字节的动态密钥
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
		//动态密钥不可用
		common_info.dynamic_key_valid = false  ; 
		memset(common_info.dynamic_key , 0 , DYNAMIC_KEY_LEN) ;
	}


	if(count_pre_sec_valid)	//每秒消息数量信息
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
	ei->station_id = ntohs(data_pack->station_id); //基站编号

	//计算报文错误码元数量
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

	word	valid_flag = ntohs(data_pack->valid_flag);//内容有效标记
	bool DZXDDY_valid = (valid_flag>>4)&0x0001 ;  //低置信度单元是否有效
	bool BWXX_valid =(valid_flag>>5)&0x0001 ; //是否已经纠正错误


	//如果动态密钥可用，则还原加密的ADS-B的14字节数据
	byte in_msg_adsb_data[14] ; 
	if(common_info.dynamic_key_valid )
	{
		for(int i=0;i<14;i++)
			in_msg_adsb_data[i] = (data_pack->message[i]) ^ (common_info.dynamic_key[i]) ; 

	}
	else
	{
		//没有动态密钥
		memcpy(in_msg_adsb_data , data_pack->message , 14);
	}


	bool src_data_soft_modifyed = false ; //源数据是否被软件修正过

	if(BWXX_valid && !DZXDDY_valid)
	{
		//信号有错误，但是硬件已经纠正
		CorrectOk = true ;
	}
	else if( !BWXX_valid && DZXDDY_valid)
	{
		//信号有错误，但是硬件没有纠正,目前不进行纠正
	//	printf("信号有错，硬件未进行纠正\rn");
		CorrectOk = false ;
	}
	else
	{
		//不可用状况
		CorrectOk = false ;
	}



	if(CorrectOk)
	{
		if(common_info.time_valid) 
		{ 
			if(ei)
				ei->gps_time_valid = true ; //硬件时间信息可用

			//如果硬件时间可用，则使用硬件时间戳
			int64_t			msg_tick_ps =get_ps_tick(data_pack->time_tick)	;//此条报文的时间计数值(ps)
			unsigned int	time_modify  = common_info.time_modify ;		//时间修正值
			byte unit_flag = (data_pack->time_tick[0]>>4)&0x0F ;			//时间单位标记
			double modify_factor = static_cast<double>(time_modify) / STD3H_HZ_MAP[unit_flag] ; //时间修正因子


			int64_t correct_time_tick_ps = 0 ; 

			if(common_info.modify_above)
			{
				//硬件时钟偏快
				correct_time_tick_ps = static_cast<int64_t>(msg_tick_ps * (1.0-modify_factor)) ; 
			}
			else
			{
				//硬件时钟偏慢
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
				ei->gps_time_valid = false ; //硬件时间信息不可用

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

	int64_t tick = it.int_value ;  //时间计数

	int  flag = (time_tick[0]>>4)&0x0F ;

	return tick* STD3H_TICK_UINT[flag] ; 

}

int64_t		std3h_fpga_decoder::get_HZ(byte flag)
{
	return STD3H_HZ_MAP[flag] ; 
}
