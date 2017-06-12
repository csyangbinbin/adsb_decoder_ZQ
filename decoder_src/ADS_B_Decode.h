/********************************************************************
created:	2011/10/09
created:	9:10:2011   10:38
filename: 	ADS_B_Decode\ADS_B_Decode.h
file path:	\ADS_B_Decode
file base:	ADS_B_Decode
file ext:	h
author:		ybb

purpose:	ADS-B MODES-S DF=17 扩展消息解码
*********************************************************************/
#ifndef __ADS_B_DECODE_INCLUDE__
#define __ADS_B_DECODE_INCLUDE__
#include <time.h>
#include <vector>

typedef struct ads_cpr_decode_param
{
	unsigned int  yz ;
	unsigned int  xz ; 
	int cpr_type ;
}ads_cpr_decode_param;


typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;
typedef float               FLOAT;
typedef double              DOUBLE;
typedef int                 INT;
typedef unsigned int        UINT;
#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif



ads_cpr_decode_param get_ads_cpr_param(BYTE ads_msg[14]) ; 

//获取DF代码
#ifndef DF_CODE
#define DF_CODE(n) ((n[0]>>3)&0x1F)
#endif

//当DF=17时获取TypeCode代码
#ifndef TypeOfDF17
#define TypeOfDF17(n)  ((n[4]>>3)&0x1F)
#endif

//ICAO地址码结构
struct    ICAOAddress
{	//使用ICAO的三个字节进行构造
	ICAOAddress(BYTE a1, BYTE a2 ,BYTE a3)
	{m_1 = a1 ;m_2 = a2,m_3 = a3 ;} 

	ICAOAddress(){m_1 = m_2 =m_3 = 0 ; }
	//转换为DWORD类型用于比较操作
	operator DWORD() const 
	{
		DWORD Value = (m_1<<16)|(m_2<<8)|(m_3) ;
		return Value ; 
	}
	// 比较操作
	bool operator<(const ICAOAddress& rvalue) 
	{
		return ((DWORD)(*this))<((DWORD)rvalue); 
	}
	BYTE m_1 ,m_2,m_3 ;
} ;

typedef struct TagADS_Time
{
	/*
	使用UTC时间
	*/
	struct tm t; /*精确到秒*/
	unsigned long tv_msec; /*毫秒*/
	unsigned long tv_wsec; /*微秒*/
	unsigned long tv_nsec; /*纳秒*/
}ADS_Time ; 

typedef struct AltitudeReplayInfo
{
		int altitude_valid;
		int altitude; 
		int altitude_unit;
}AltitudeReplayInfo;

/**/
/*
DF=17 ,TypeCode=1,2,3,4
报文类型:Aircraft Identification and Category
*/

//飞行器识别码与类型信息
#define    CategorySetA     4
#define    CategorySetB     3
#define    CategorySetC     2
#define    CategorySetD     1 
typedef struct TagIdentificationInfo
{
	char FlightID[8] ;  //飞行器识别码
	BYTE CategorySet ; //飞行器分类(A,B,C,D)
	BYTE Category    ; //飞行器详细分类
	BYTE OriFlightID[6] ;//原始的6字节的ID
}IdentificationInfo,*PIdentificationInfo ; 


//_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-朴素的分割线

/*
DF=17 , TypeCode=5,6,7,8
报文类型:Surface Position Message

DF=17, TypeCode=9,10,11,12,13,14,15,16,17,18
报文类型：Airborne Position Message

飞行器位置类型
"Surface Position Message"报文解码为地面位置
"Airborne Position Message"报文解码为空中位置
*/
typedef enum TagLocationType
{
	Air , Surface
}LocationType ; 

typedef enum TagAltitudeType
{
Baro , /*DF=17 TypeCode={9,18} 高度为气压高度*/
HAE    /*DF=17 TypeCode={20,22} 高度为几何高度*/
}AltitudeType ; 
//定义飞行器的位置精度版本
#define VERSION_ZERO      0
#define VERSION_ONE       1
#define VERSION_NULL      3

//不可用的精度级别
#define NULL_PERCISION 100

#define INVALID_T 3
#define INVALID_Q 3

//位置导航精度级别未知
#define UNKNOWN_NACP 0xFF 

//位置精度信息
typedef struct tagPosPercision
{
	DWORD PercisionVer ; //位置精度版本
	DWORD NUC ;          //VER 0 时使用的NUC <Version==0 时可用>
	DWORD NIC ;         //VER 1 时使用的NIC <Version==1 时可用>
	BYTE NIC_SUPPLEMENT_A ; //NIC补充位(VER ==1 时用于确定位置精度NIC) <Version==1 时可用>
	BYTE NIC_SUPPLEMENT_B ; //NIC补充位(VER ==1 时用于确定位置Rc) <Version==1 时可用>
	BYTE NACP             ;//位置导航精度级别(在DF=17,TypeCode=31 的消息中传输)
}PosPercision , *PPosPercision ; 

//飞行器位置信息定义
typedef struct TagAircraftLocationInfo
{
	INT 	       Longitude[2];    
	INT 	       Latitude[2];     
	INT 	       Altitude;	   //高度(单位：英尺 ft)
	AltitudeType   AltType ;       //高度类型
	DOUBLE	       EvalLon;        //经度
	DOUBLE         EvalLat;         //纬度 
	LocationType   LocType;        //位置类型
	PosPercision   LocPercision;   //位置精度
	BYTE SurveillanceStatus  ;     //空中位置信息中的监视状态
	BYTE T ;                       //时间同步标记(0--未同步 ， 1--同步)
	/*
	对于Q位的说明
	1. Barometric altitude encoded in 25 (Q==1) or 100 (Q==0) foot increments (as indicated by the
	Q Bit) or,
	2. GNSS height above ellipsoid (HAE).
	Note: GNSS altitude MSL is not accurate enough for use in the position report.
	*/
	BYTE Q ;                         //高度类型
	BOOL LocationAvailable ;         //位置信息可否可用
}AircraftLocationInfo , *PAircraftLocationInfo ; 


/*
定义飞行器地面状态

类型为"Surface Position Message"的报文中除了包含目标地面位置信息外，还包含移动速度与航向信息
*/
typedef struct TagSurfacePosInfo
{

	DOUBLE Movement ;  //地面运行速度(单位：千米/小时 km/h)
	BYTE MoveMentCode ; //地面移动速度编码
	/*

	地面移动速度编码数值所代表的含义

	0                            No Movement Information Available
	1                            Aircraft Stopped (Ground Speed = 0 knots)
	2                            0 knots < Ground Speed ≤ 0.2315 km/h (0.125 kt)
	3 - 8                        0.2315 km/h (0.125 kt) < Ground Speed ≤ 1.852 km/h (1 kt) 0.2700833 km/h steps
	9 - 12                       1.852 km/h (1 kt) < Ground Speed ≤ 3.704 km/h (2 kt) 0.463 km /h (0.25 kt) steps
	13 - 38                      3.704 km/h (2 kt) < Ground Speed ≤ 27.78 km/h (15 kt) 0.926 km/h (0.50 kt) steps
	39 - 93                      27.78 km/h (15 kt) < Ground Speed ≤ 129.64 km/h (70 kt) 1.852 km/h (1.00 kt) steps
	94 - 108                     129.64 km/h (70 kt) < Ground Speed ≤ 185.2 km/h (100 kt) 3.704 km/h (2.00 kt) steps
	109 - 123                    185.2 km/h (100 kt) < Ground Speed ≤ 324.1 km/h (175 kt) 9.26 km/h (5.00 kt) steps
	124                          324.1 km/h (175 kt) < Ground Speed
	125                          Reserved for Aircraft Decelerating
	126                          Reserved for Aircraft Accelerating
	127                          Reserved for Aircraft Backing-Up
	*/
	BYTE HeadingState ;//方位状态( 0--方位不可用 1--方位可用)
	DOUBLE Heading ;   //方位(单位:角度)
}SurfacePosInfo , *PSurfacePosInfo;
//_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-朴素的分割线
/*
DF=17 , TypeCode=19
报文类型:Airborne Velocity Message
*/

//定义飞行器空中速度信息结构
typedef struct TagVelocityInfo
{


	BYTE SubType ;         //子类型 (GS/AS)
	//1 = GS/Normal (速度在1000节以下)
	//2 = GS/Supersonic (速度在1022节以上)
	//3 = Airspeed/Heading/Normal (亚音速飞行器)
	//4 = Airspeed/Heading/Supersonic (保留为空速超过1000节的飞行器)

	BOOL Supersonic ;                     //超音速标志(GS/AS)
	BYTE IntentChangeFlag ;               //Intent Change Flag (0--标志没有改变 1--标志改变)(GS/AS)
	BYTE NACv ;                           //航行精度级别(GS/AS)
	BYTE Dire_w ;                         //地速东西方向(0--向东 1--向西)(GS)
	UINT Velocitye_w ;					 //东西速度(单位:节 kts)(GS)
	BYTE Dirn_s;                          //地速南北方向(0--向北 1--向南)(GS)
	UINT  Velocityn_s;					 //南北速度(单位:节 kts)(GS)
	DOUBLE GS ;                           //地速(单位:节 kts)(GS)
	DOUBLE GSHeading ;                    //地速方向(GS)
	BOOL HdgAvailStatusBit ;              //航向状态标记(false--空速航向信息不可用 true--空速航行信息可用)(AS)
	DOUBLE ASHeading;                      //空速方向(AS)
	BOOL TASFlag ;                        //true--空速为真空速 ，false--空速为指示空速(AS)
	INT IAS ;                             //指示空速(单位:kts)(AS)
	INT TAS;                              //真空速(单位:节 kts)(AS)
	BYTE VRateSourceBit ;                 //Source Bit For Vertical Rate( 0--Geometric 1--Baro)(GS/AS)
	BYTE CurState ;                       //飞行状态(0-爬升   1-下降)(GS/AS)
	INT VRate ;                           //升速(单位: 英尺/分钟 ft/min) ,正值为上升，负值为下降(GS/AS)
	INT GPSAltDiff ;                      //GPS高度与气压高度的修正值(正值说明GPS高度大于气压高度，负值为低于气压高度)(GS/AS)    

	BOOL GSInfoAvailable ;                //地速相关信息是否可用
	BOOL ASInfoAvailable ;                //空速相关信息是否可用


}VelocityInfo,*PVelocityInfo;
//////////////////////////////////////////////////////////////////////////朴素的分割线
typedef struct TagOperationalStatusInfo
{
BYTE HDR ; //水平方参考方向( 1--磁场北  0--正北)
}OperationalStatusInfo ; 
//_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-朴素的分割线
//定义飞行器相关信息
typedef struct AircraftInfo
{
	ICAOAddress                ICAOID ;                     //ICAO地址
	IdentificationInfo         IdAndCategory;              //飞行器身份识别
	AircraftLocationInfo       Location ;                  //位置信息
	SurfacePosInfo             SurfaceState;               //与地面位置相关的信息
	VelocityInfo               VelocityState;              //空中速度相关信息
	OperationalStatusInfo      StatusInfo ;                //飞行状态信息
	ADS_Time                   LocationLastUpdateTime ;    //位置信息最后的更新时间
	ADS_Time                   InfoLastUpdateTime ;        //收到关于此飞行器的数据的最后时间
	BOOL                       bTimeOut;                   //飞行器目标超时(长时间未收到此目标的任何信息)
	unsigned int			   squawk;						//应答机编号
	AltitudeReplayInfo		   AltitudeReplay;				//高度应答回复
}AircraftInfo,*PAircraftInfo ; 





typedef void* LPVOID ; 
typedef LPVOID DE_HANDLE ; 
typedef void ( * pfnVisitCallBack)(const AircraftInfo* );
#define INVALID_DECODE_HANDLE  0

//////////////////////////////////////////////////////////////////////////朴素的分割线

/*
创建新的解码器实例
返回值:解码器实例的句柄,失败返回INVALID_DECODE_HANDLE
*/
DE_HANDLE InitNewDecoder(double home_lon , double home_lat ) ; 

/*
设置与解码器相关的中心位置信息
*/
void SetDecoderHome(DE_HANDLE deHandle ,double lon , double lat) ;

/*
解码DF=17的ADS-B扩展信息
返回值，如果成功解码出位置信息返回TRUE， 否则返回FALSE
*/
BOOL DecodeADSMessage(DE_HANDLE deHandle ,  // [IN] 解码器句柄
					  BYTE AdsMessage[14], // [IN] ADS消息报文
					  ADS_Time Time       // [IN] 报文接收时间
					  ) ;

/*
解码空中位置信息
返回值，如果成功解码出位置信息返回TRUE， 否则返回FALSE
*/
BOOL DecodeAirPostion(DE_HANDLE deHandle ,  // [IN] 解码器句柄
					  BYTE AdsMessage[14], // [IN] ADS消息报文
					  ADS_Time Time       // [IN] 报文接收时间
					  ) ;


/*
解码地面位置信息
返回值，如果成功解码出位置信息返回TRUE， 否则返回FALSE
*/
BOOL  DecodeSurfacePostion(DE_HANDLE deHandle ,  // [IN] 解码器句柄
						   BYTE AdsMessage[14], // [IN] ADS消息报文
						   ADS_Time Time       // [IN] 报文接收时间
						   ) ;

/*
解码速度信息
返回值:如果解码成功返回TRUE，否则返回FALSE
*/
BOOL DecodeVelocity(DE_HANDLE deHandle ,  // [IN] 解码器句柄
					BYTE AdsMessage[14],    // [IN] ADS消息报文
					ADS_Time Time          // [IN] 报文接收时间
					);

/*
解析飞机识别码与类型信息
返回值:无
*/
BOOL DecodeIdAndCategoryInfo(DE_HANDLE deHandle ,  // [IN] 解码器句柄
							 BYTE AdsMessage[14],  // [IN] ADS长消息报文
							 ADS_Time Time         // [IN] 报文接收时间
							 );
/*
工具函数
解析飞机详细类型(返回类型字符串的长度)
返回值:类型字符串的长度,如果不存在此类别则返回NULL
*/
const char* GetAircraftCategoryDescription(
	BYTE CategorySet ,  // [IN] 飞行器总类别(A,B,C,D)
	BYTE Category      // [IN] 飞行器详细类别
	);

/*
解析飞机的状态信息
返回值:如果解码成功返回TRUE，否则返回FALSE
*/
BOOL DecodeOperationalStatus(DE_HANDLE deHandle ,  // [IN] 解码器句柄
							 BYTE AdsMessage[14],  // [IN] 112位ADS长消息报文
							 ADS_Time Time         // [IN] 报文接收时间
							 ) ; 
/*
返回正在进行解码的目标数量
*/
size_t GetTotalCount(DE_HANDLE deHandle)  ; 

/*
返回指定的ICAO的目标的相关信息
*/
AircraftInfo* GetAircraftInfo(DE_HANDLE deHandle , ICAOAddress ICAO);

/*
检查超过指定时间而没有任何更新的目标
返回值:超时的目标的数量
*/
size_t CheckTimeOut(DE_HANDLE deHandle,// [IN] 解码器句柄
					std::vector<ICAOAddress>& TimeOutAircrafts, //[OUT] 存储超时的目标的ICAO
					ADS_Time	RefTime ,  //  [IN] 计算超时的参考时间点
					int TimeOutSeconds,    //  [IN] 超时时长（单位:秒）
					BOOL bSign             //  [IN] 是否标记超时的目标，如果标记，则删除超时目标操作会将其删除
					);

/*
在飞行器信息数据表中删除所有超时的条目 , 在调用前先调用CheckTimeOut函数检查超时信息条目
返回值：删除的条目的数量
*/
size_t DeleteTimeOut(DE_HANDLE deHandle// [IN] 解码器句柄
					 ) ; 
size_t DeleteTimeOutFromRefTime(DE_HANDLE deHandle , ADS_Time     RefTime ,  int TimeOutSeconds  ) ;

/*
访问解码器中所有信息
注:访问函数应尽可能短小 ，因为VisitAll函数内部使用读写锁，如长时间占用会造成其他线程长时间等待
*/
void VisitAll(DE_HANDLE deHandle , // [IN] 解码器句柄
			 pfnVisitCallBack fn  // [IN] 遍历函数
			 ) ;


double real_lon(double lon ,double home_lon)  ; 
double real_lat(double lat , double home_lat)  ;
#endif
