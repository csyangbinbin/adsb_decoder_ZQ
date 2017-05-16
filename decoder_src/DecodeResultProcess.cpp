
#include "DecodeResultProcess.h"
#include "ADSB_DataProtocol.h"
#include "utility.h"
#include <string.h>
#include <cstdio>
#include <cstdlib>
#include "../debug.h"

ADSB_ResultProcess::ADSB_ResultProcess(adsb_core_decoder* ads_decoder) :
m_deHandle(ads_decoder)
{

}
ADSB_ResultProcess::~ADSB_ResultProcess()
{

}

bool	ADSB_ResultProcess::Init()
{
	return true;
}

adsb_decode_result_t
	ADSB_ResultProcess::ProcessResult(DWORD DecodeResult, BYTE ADS_B_MSG[14],
	const ADS_Time & timestamp, extra_info* ei)
{

	adsb_decode_result_t result ; 
	memset(&result ,0 ,sizeof(adsb_decode_result_t));
	result.decode_type =decode_error_type ; 



	ICAOAddress ICAO = GetICAO(ADS_B_MSG);

	AircraftInfo* info =m_deHandle->get_aircraft_info(ICAO);

	if (ERR_DECODE_TYPE == DecodeResult)
		return result;
	else if (ID_DECODE_TYPE == DecodeResult)
	{

		return 	ProcessID_Pack(info, ADS_B_MSG, timestamp ,ei);

	}
	else if (SP_DECODE_TYPE == DecodeResult)
	{
		return	ProcessSPLocation_Pack(info, ADS_B_MSG, timestamp , ei);
	}
	else if (AP_DECODE_TYPE == DecodeResult)
	{

		return ProcessLocation_Pakc(info, ADS_B_MSG, timestamp , ei);

	}
	else if (VEL_DECODE_TYPE == DecodeResult)
	{
		return 	ProcessVel_Pack(info, ADS_B_MSG, timestamp ,ei);

	}
	else
	{
		return	result ; 
	}

}


static void   PackCommonInfo(adsb_common_info_t* comm_info ,
	const ADS_Time & timestamp , 
	ICAOAddress	ICAOID,  
	extra_info* ei )
{

	memset((void*)comm_info , 0 ,sizeof(adsb_common_info_t));
	//ICAO地址
	comm_info->icao_addr.m1 = ICAOID.m_1 ;
	comm_info->icao_addr.m2 = ICAOID.m_2 ;
	comm_info->icao_addr.m3	= ICAOID.m_3 ;

	//时间信息
	comm_info->timestamp.valid = ei->gps_time_valid ; 

	if(comm_info->timestamp.valid)
	{
		//硬件时间信息可用
	comm_info->timestamp.tm_year =	timestamp.t.tm_year ; 
	comm_info->timestamp.tm_mon =	timestamp.t.tm_mon ; 
	comm_info->timestamp.tm_mday =	timestamp.t.tm_mday ; 
	comm_info->timestamp.tm_hour = timestamp.t.tm_hour ; 
	comm_info->timestamp.tm_min	 = timestamp.t.tm_min ; 
	comm_info->timestamp.tm_sec	 = timestamp.t.tm_sec ; 
	comm_info->timestamp.tv_msec = timestamp.tv_msec ; 
	comm_info->timestamp.tv_wsec = timestamp.tv_wsec ; 
	comm_info->timestamp.tv_nsec = timestamp.tv_nsec ; 
	}

	//Station ID
	comm_info->station_id = ei->station_id;
}

adsb_decode_result_t
	ADSB_ResultProcess::ProcessID_Pack(const AircraftInfo* info,
	BYTE ADS_B_MSG[14], const ADS_Time & timestamp,  extra_info* ei )
{
	adsb_decode_result_t result ;
	memset(&result , 0 , sizeof(adsb_decode_result_t) ) ;
	result.decode_type = identification_type ; 
	//填充通用信息
	PackCommonInfo(&result.id_info_.comm_info ,timestamp ,info->ICAOID , ei);
	

	memcpy(result.id_info_.flight_id, info->IdAndCategory.FlightID, 8);

	result.id_info_.category = info->IdAndCategory.Category;
	result.id_info_.category_set =  info->IdAndCategory.CategorySet ; 

	return	result;
}



adsb_decode_result_t ADSB_ResultProcess::ProcessSPLocation_Pack( const AircraftInfo* info ,BYTE ADS_B_MSG[14] ,
	const ADS_Time & timestamp ,  extra_info* ei) 
{
	adsb_decode_result_t result ;
	memset(&result , 0 , sizeof(adsb_decode_result_t) ) ;


	if (!info->Location.LocationAvailable)
	{
		log_print("SP > info->Location.LocationAvailable == false\r\n");
		result.decode_type = decode_error_type ; 
		return result; 
	}

	result.decode_type = surface_location_type ;

	//填充通用信息
	PackCommonInfo(&result.surface_location_info_.comm_info ,timestamp ,info->ICAOID , ei);
	

	result.surface_location_info_.longitude = info->Location.EvalLon ; 
	result.surface_location_info_.latitude = info->Location.EvalLat ;
	result.surface_location_info_.movement = info->SurfaceState.Movement;
	result.surface_location_info_.heading_state = info->SurfaceState.HeadingState ;
	result.surface_location_info_.heading_value = info->SurfaceState.Heading ; 	

	return result ; 
}


adsb_decode_result_t
	ADSB_ResultProcess::ProcessLocation_Pakc(const AircraftInfo* info,
	BYTE ADS_B_MSG[14], const ADS_Time & timestamp , extra_info* ei)
{

	adsb_decode_result_t result ;
	memset(&result , 0 , sizeof(adsb_decode_result_t) ) ;

	if (!info->Location.LocationAvailable)
	{
		log_print("AP > info->Location.LocationAvailable == false\r\n");
		result.decode_type = decode_error_type ; 
		return result; 
	}

	result.decode_type = air_location_type ;

	//填充通用信息
	PackCommonInfo(&result.air_location_info_.comm_info ,timestamp ,info->ICAOID , ei);
	

	result.air_location_info_.longitude = info->Location.EvalLon ;  //经度
	result.air_location_info_.latitude = info->Location.EvalLat ;	//纬度

	double GPSAltDiff = 0 ;
	if (info->VelocityState.GSInfoAvailable || info->VelocityState.ASInfoAvailable)
	{
		GPSAltDiff = info->VelocityState.GPSAltDiff *0.3048;
	}

	result.air_location_info_.altitude = info->Location.Altitude * 0.3048  + GPSAltDiff;  //GPS高度

	return	result ;
}

adsb_decode_result_t
	ADSB_ResultProcess::ProcessVel_Pack(const AircraftInfo* info,
	BYTE ADS_B_MSG[14], const ADS_Time & timestamp,  extra_info* ei )
{
	adsb_decode_result_t result ;
	memset(&result , 0 , sizeof(adsb_decode_result_t) ) ;


	if (!info->VelocityState.GSInfoAvailable)
	{
		log_print("VL > info->VelocityState.GSInfoAvailable == false\r\n");
		result.decode_type = decode_error_type ; 
		return result; 
	}

	if (!(info->VelocityState.SubType == 1 || info->VelocityState.SubType == 2))
	{
		log_print("VL > info->VelocityState.SubType=%d\r\n" , info->VelocityState.SubType);

		result.decode_type = decode_error_type ; 
		return result;
	}

	result.decode_type = velocity_type ;
	//填充通用信息
	PackCommonInfo(&result.velocity_info_.comm_info ,timestamp ,info->ICAOID , ei);
	


	result.velocity_info_.gs_value = info->VelocityState.GS ; 
	result.velocity_info_.gs_heading = info->VelocityState.GSHeading ; 
	result.velocity_info_.vrate = info->VelocityState.VRate ; 

	return result ;
}

adsb_decode_result_t ADSB_ResultProcess::ProcessDevInfo(extra_info* ei)
{

	/*
	u16							valid_flag ;			
	u16							station_id ;			///< 接收机编号
	dev_board_gps_info_t		gps_info;				///< GPS/BD定位信息
	dev_board_battery_info_t	battery_info;			///< 电池信息	
	double						board_temperature;		///< 设备温度
	dev_channel_state_info_t	channel_state ;			///< 信号处理通道状态
	u8							dev_pcb_version;		///< 设备硬件电路版本号
	u8							dev_firmware_version ;	///< 设备硬件固件版本号

	*/


	adsb_decode_result_t result ;
	memset(&result , 0 , sizeof(adsb_decode_result_t) ) ;
	result.decode_type = dev_board_info_type ;

	const fpga_board_info& info = ei->hd_info ;

	result.board_info_.valid_flag = info.valid_flag; //主机字节序
	result.board_info_.station_id = info.station_id ;  //接收机编号

	result.board_info_.gps_info.satellite_count = info.gps_info.gps_state ;//搜星数量
	result.board_info_.gps_info.longitude	=	info.gps_info.longitude / 1000000.0 ;
	result.board_info_.gps_info.latitude	=	info.gps_info.latitude /1000000.0;
	result.board_info_.gps_info.altitude	=	info.gps_info.altitude ;


	if(info.vel_info.vel_value == 0xFFFFFFFF)
	{
	//GPS速度信息无效
		result.board_info_.gps_info.velocity_heading		=		0 ;
		result.board_info_.gps_info.velocity_value			=		0;
	}
	else
	{
		result.board_info_.gps_info.velocity_heading		=	info.vel_info.vel_heading /10.0 ;
		result.board_info_.gps_info.velocity_value			=	info.vel_info.vel_value /10.0 ; 
	}

	//GPS时间信息
	result.board_info_.gps_info.year  =info.current_time.year +2000;
	result.board_info_.gps_info.month  =info.current_time.month ;
	result.board_info_.gps_info.day  =info.current_time.day ;
	result.board_info_.gps_info.hour  =info.current_time.hour ;
	result.board_info_.gps_info.minute  =info.current_time.minute ;
	result.board_info_.gps_info.second  =info.current_time.second ;

	//电池信息
	result.board_info_.battery_info.state = info.battery_info.state ;
	result.board_info_.battery_info.capability = info.battery_info.capability ;
	result.board_info_.battery_info.voltage = info.battery_info.voltage / 10.0 ;
	result.board_info_.battery_info.current = info.battery_info.current/10.0 ;
	result.board_info_.battery_info.work_time = info.battery_info.work_time ;
	result.board_info_.battery_info.battery_temperature = info.battery_info.battery_temperature ;

	//设备温度信息
	result.board_info_.board_temperature = info.temperature_info.board_temperature ; 
	
	//通道状态
	result.board_info_.channel_state.channel_id = info.c_state.id ; 
	if(info.c_state.state > 2)
	{
	result.board_info_.channel_state.channel_state = ADSB_CHANNEL_STATE_UNDEFINED ; 
	}
	else
	{
		result.board_info_.channel_state.channel_state  = info.c_state.state ;
	}
	
	//电路版本
	result.board_info_.dev_pcb_version = info.hd_version ; 

	//固件版本
	result.board_info_.dev_firmware_version = info.hd_sf_version ; 

	return result ;
}

