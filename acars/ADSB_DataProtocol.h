#ifndef __ADSB_DATA_PROTOCOL_INCLUDE__
#define __ADSB_DATA_PROTOCOL_INCLUDE__

typedef unsigned char u8;
typedef char s8;
typedef unsigned short u16;
typedef short s16;
typedef unsigned int u32;
typedef int s32;


/** @defgroup adsparm ADS-B目标状态改变消息参数
*  ADS-B目标状态改变消息参数
*  @{
*/



/*! \def ADS_B_ID_TAG
\brief ADS-B目标ID信息标签
*/
#define ADS_B_ID_TAG        0xFF01
/*! \def ADS_B_LOC_TAG
\brief ADS-B空中位置信息标签
*/
#define ADS_B_LOC_TAG       0xFF02
/*! \def ADS_B_VEL_TAG
\brief ADS-B速度信息标签
*/
#define ADS_B_VEL_TAG       0xFF03
/*! \def ADS_B_OP_TAG
\brief ADS-B操作状态信息标签
*/
#define ADS_B_OP_TAG        0xFF04
/*! \def ADS_B_SURFACE_TAG
\brief ADS-B地面信息标签
*/
#define ADS_B_SURFACE_TAG   0xFF09
/*! \def ADS_B_SIGNAL_TAG
\brief ADS-B信号信息标签
*/
#define ADS_B_SIGNAL_TAG    0xFF05
/*! \def ADS_B_SIGNAL_TAG
\brief 硬件连接断开标签
*/
#define IS_HD_CONNECT_TAG   0xFF06

/*! \def ACARS_DATA_TAG
\brief ACARS数据标签
*/
#define ACARS_DATA_TAG		0xFF07

/*! \def ADS_B_TDAZIMUTHINFO_TAG
\brief 时差测向信息


*/
#define ADS_B_TDAZIMUTHINFO_TAG   0xFF10
/*! \def VERSION_ZERO
\brief 精度级别版本-0
*/

#define VERSION_ZERO      0

/*! \def VERSION_ONE
\brief 精度级别版本-1
*/
#define VERSION_ONE       1

/*! \def VERSION_NULL
\brief 精度级别版本未知
*/
#define VERSION_NULL      3

#pragma pack(1)

/*!
* @brief 年月日时分秒时间表示
*
* 年月日时分秒时间表示
*/
struct TM
{
	u32 tm_sec;		///< Seconds.     [0-60] (1 leap second) 
	u32 tm_min;		///< Minutes.     [0-59] 
	u32 tm_hour;	 ///< Hours.       [0-23] 
	u32 tm_mday;	///< Day.         [1-31] 
	u32 tm_mon;		///< Month.       [0-11] 
	u32 tm_year;	///< Year - 1900.  
	u32 tm_wday;	///< Day of week. [0-6] 
	u32 tm_yday;	///< Days in year.[0-365] 
	u32 tm_isdst;	///< DST.         [-1/0/1]
};

/*!
* @brief ADS-B目标ICAO编号表示
*
* ADS-B目标ICAO编号表示
*/
typedef struct TagICAO
{
	u8 m1; ///< 高位字节
	u8 m2; ///< 一字节
	u8 m3; //< 低位字节
} ICAO;

/*!
* @brief ADS-B消息通用结构
*
* ADS-B消息通用结构
*/
typedef struct TagCommInfo
{
	u16 Tag;			///< 命令标签
	ICAO Addr;			///< ICAO地址
	struct TimeInfo
	{
		struct TM t;	///< 时间信息(年月日时分秒)
		u32 tv_msec;	///< 毫秒
		u32 tv_nsec;	///< 纳秒
	} Timestamp;		///< 时间戳
	u8 ADSB_MSG[14];	 ///< 原始的ADS-B报文信息
} CommInfo;


/*!
* @brief 飞行器的身份识别信息
*
*	飞行器的身份识别信息
*/
typedef struct TagADSBIdentificationInfo
{
	CommInfo Comm_info;	///< 通用信息
	s8 FlightID[8];		///< ID名称
	u8 CategorySet;		///< 类别集
	u8 Category;		///< 类别
} ADSBIdentificationInfo;


/*!
* @brief ADS-B位置精度信息
*
*	ADS-B位置精度信息
*/
typedef struct tagADSBPosPercision
{
	u8 Version;			///<精度版本
	union
	{
		u8 NUC_Value;	///< NUC
		struct
		{
			u8 NIC;		///<  NIC
			u8 NIC_SUPPLEMENT_A; ///< 补充值A
			u8 NIC_SUPPLEMENT_B; ///< 补充值B
		} NIC_Value;	///< NIC
	} Value;			///< 位置精度值
	u8 NACP;			///< 位置导航精度级别(在DF=17,TypeCode=31 的消息中传输)
} ADSBPosPercision;

/*!
* @brief ADS-B位置信息
*
*	ADS-B位置信息
*/
typedef struct TagADSBLocationInfo
{
	CommInfo Comm_info;				///< 通用信息
	ADSBPosPercision PosPercision;	///< 位置精度信息
	s32 Alt;						///< 高度(单位:M)
	s32 Lon;						///< 经度(单位:0.0000001角度)
	s32 Lat;						///< 纬度(单位:0.0000001角度)
	s32 GPSAltDiff;					///< GPS高度与气压高度的修正值(正值说明GPS高度大于气压高度，负值为低于气压高度 , 单位:M)

} ADSBLocationInfo;

/*!
* @brief 速度信息补充
*
*	速度信息补充
*/
typedef struct TagADSBVelocityInfoMisc
{
	u8 SubType;
	u8 NACv;
	u8 Dire_w;				 ///< 地速东西方向(0--向东 1--向西)(GS)
	u32 Velocitye_w;		 ///< 东西速度(单位:节 kts)(GS)
	u8 Dirn_s;				 ///< 地速南北方向(0--向北 1--向南)(GS)
	u32 Velocityn_s;		 ///< 南北速度(单位:节 kts)(GS)
	u8 VRateSourceBit;		 ///< 源

} ADSBVelocityInfoMisc;

/*!
* @brief ADS-B速度信息	
*
*	ADS-B速度信息	
*/
typedef struct TagADSBVelocityInfo
{
	CommInfo Comm_info;				///< 通用信息
	u32 GS;							///< 地速(单位:节 kts)
	u32 GSHeading;					///< 地速方向(单位:0.01角度) (0xFFFFFFFF表示无效)
	s32 VRate;						///< 升速(单位: 英尺/分钟 ft/min) ,正值为上升，负值为下降
	s32 GPSAltDiff;					///< GPS高度与气压高度的修正值(正值说明GPS高度大于气压高度，负值为低于气压高度 , 单位:M)
	ADSBVelocityInfoMisc Misc ;		///< 附加信息
} ADSBVelocityInfo;

/*!
* @brief ADS-B地面信息
*
*	ADS-B地面信息	
*/
typedef struct TagADSSurfaceInfo
{
	CommInfo Comm_info;				///< 通用信息
	ADSBPosPercision PosPercision ; ///< 位置精度信息
	s32 Lon;						///< 经度(单位:0.0000001角度)
	s32 Lat;						///< 纬度(单位:0.0000001角度)
	u32 Movement ;					///< 地面运行速度(单位：0.01千米/小时 km/h)
	u8  HeadingState ;				///< 方位状态( 0--方位不可用 1--方位可用)
	u32 Heading ;					///< 方位(单位:0.01 角度)
} ADSSurfaceInfo;


/*!
* @brief ADS-B操作状态信息
*
*	ADS-B操作状态信息
*/
typedef struct TagADSBOperationalStatusInfo
{
	CommInfo Comm_info;		///< 通用信息
} ADSBOperationalStatusInfo; ///< 状态信息

/*!
* @brief ADS-B信号相关信息
*
*	ADS-B信号相关信息
*/
typedef struct TagSignalInfo
{
    CommInfo Comm_info;
    u8 snr ; //信噪比
    u8 pf[4] ; //频谱信息
} ADSBSignalInfo; 

/*!
* @brief 硬件是否连接信息
*
*	硬件是否连接信息
*/
typedef struct TagNaNConnect  //硬件连接断开
{
    u16 Tag;		// 命令标签
    u8  conn_state ; //连接状态
} NaNConnect;

/*!
* @brief 双站短基线测向信息
*
*    双站短基线测向信息
*/
typedef struct TagTDAzimuthInfo  //硬件连接断开
{
    CommInfo Comm_info;
    double Azimuth1 ; //时差测向方位值1(角度)
    double Azimuth2 ; //时差测向方位值2(角度)
} TDAzimuthInfo;

/*!
* @brief ACARS数据信息
*
*	ACARS数据信息	
*/
typedef struct TagACARSInfo
{
    CommInfo Comm_info;				///< 通用信息
    u8		 data[244];				///< ACARS原始数据
} ACARSInfo;

#pragma pack()


/** @} */ // end of adsparm


#endif