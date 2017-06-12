/********************************************************************
created:	2011/10/09
created:	9:10:2011   10:38
filename: 	ADS_B_Decode\ADS_B_Decode.h
file path:	\ADS_B_Decode
file base:	ADS_B_Decode
file ext:	h
author:		ybb

purpose:	ADS-B MODES-S DF=17 ��չ��Ϣ����
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

//��ȡDF����
#ifndef DF_CODE
#define DF_CODE(n) ((n[0]>>3)&0x1F)
#endif

//��DF=17ʱ��ȡTypeCode����
#ifndef TypeOfDF17
#define TypeOfDF17(n)  ((n[4]>>3)&0x1F)
#endif

//ICAO��ַ��ṹ
struct    ICAOAddress
{	//ʹ��ICAO�������ֽڽ��й���
	ICAOAddress(BYTE a1, BYTE a2 ,BYTE a3)
	{m_1 = a1 ;m_2 = a2,m_3 = a3 ;} 

	ICAOAddress(){m_1 = m_2 =m_3 = 0 ; }
	//ת��ΪDWORD�������ڱȽϲ���
	operator DWORD() const 
	{
		DWORD Value = (m_1<<16)|(m_2<<8)|(m_3) ;
		return Value ; 
	}
	// �Ƚϲ���
	bool operator<(const ICAOAddress& rvalue) 
	{
		return ((DWORD)(*this))<((DWORD)rvalue); 
	}
	BYTE m_1 ,m_2,m_3 ;
} ;

typedef struct TagADS_Time
{
	/*
	ʹ��UTCʱ��
	*/
	struct tm t; /*��ȷ����*/
	unsigned long tv_msec; /*����*/
	unsigned long tv_wsec; /*΢��*/
	unsigned long tv_nsec; /*����*/
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
��������:Aircraft Identification and Category
*/

//������ʶ������������Ϣ
#define    CategorySetA     4
#define    CategorySetB     3
#define    CategorySetC     2
#define    CategorySetD     1 
typedef struct TagIdentificationInfo
{
	char FlightID[8] ;  //������ʶ����
	BYTE CategorySet ; //����������(A,B,C,D)
	BYTE Category    ; //��������ϸ����
	BYTE OriFlightID[6] ;//ԭʼ��6�ֽڵ�ID
}IdentificationInfo,*PIdentificationInfo ; 


//_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-���صķָ���

/*
DF=17 , TypeCode=5,6,7,8
��������:Surface Position Message

DF=17, TypeCode=9,10,11,12,13,14,15,16,17,18
�������ͣ�Airborne Position Message

������λ������
"Surface Position Message"���Ľ���Ϊ����λ��
"Airborne Position Message"���Ľ���Ϊ����λ��
*/
typedef enum TagLocationType
{
	Air , Surface
}LocationType ; 

typedef enum TagAltitudeType
{
Baro , /*DF=17 TypeCode={9,18} �߶�Ϊ��ѹ�߶�*/
HAE    /*DF=17 TypeCode={20,22} �߶�Ϊ���θ߶�*/
}AltitudeType ; 
//�����������λ�þ��Ȱ汾
#define VERSION_ZERO      0
#define VERSION_ONE       1
#define VERSION_NULL      3

//�����õľ��ȼ���
#define NULL_PERCISION 100

#define INVALID_T 3
#define INVALID_Q 3

//λ�õ������ȼ���δ֪
#define UNKNOWN_NACP 0xFF 

//λ�þ�����Ϣ
typedef struct tagPosPercision
{
	DWORD PercisionVer ; //λ�þ��Ȱ汾
	DWORD NUC ;          //VER 0 ʱʹ�õ�NUC <Version==0 ʱ����>
	DWORD NIC ;         //VER 1 ʱʹ�õ�NIC <Version==1 ʱ����>
	BYTE NIC_SUPPLEMENT_A ; //NIC����λ(VER ==1 ʱ����ȷ��λ�þ���NIC) <Version==1 ʱ����>
	BYTE NIC_SUPPLEMENT_B ; //NIC����λ(VER ==1 ʱ����ȷ��λ��Rc) <Version==1 ʱ����>
	BYTE NACP             ;//λ�õ������ȼ���(��DF=17,TypeCode=31 ����Ϣ�д���)
}PosPercision , *PPosPercision ; 

//������λ����Ϣ����
typedef struct TagAircraftLocationInfo
{
	INT 	       Longitude[2];    
	INT 	       Latitude[2];     
	INT 	       Altitude;	   //�߶�(��λ��Ӣ�� ft)
	AltitudeType   AltType ;       //�߶�����
	DOUBLE	       EvalLon;        //����
	DOUBLE         EvalLat;         //γ�� 
	LocationType   LocType;        //λ������
	PosPercision   LocPercision;   //λ�þ���
	BYTE SurveillanceStatus  ;     //����λ����Ϣ�еļ���״̬
	BYTE T ;                       //ʱ��ͬ�����(0--δͬ�� �� 1--ͬ��)
	/*
	����Qλ��˵��
	1. Barometric altitude encoded in 25 (Q==1) or 100 (Q==0) foot increments (as indicated by the
	Q Bit) or,
	2. GNSS height above ellipsoid (HAE).
	Note: GNSS altitude MSL is not accurate enough for use in the position report.
	*/
	BYTE Q ;                         //�߶�����
	BOOL LocationAvailable ;         //λ����Ϣ�ɷ����
}AircraftLocationInfo , *PAircraftLocationInfo ; 


/*
�������������״̬

����Ϊ"Surface Position Message"�ı����г��˰���Ŀ�����λ����Ϣ�⣬�������ƶ��ٶ��뺽����Ϣ
*/
typedef struct TagSurfacePosInfo
{

	DOUBLE Movement ;  //���������ٶ�(��λ��ǧ��/Сʱ km/h)
	BYTE MoveMentCode ; //�����ƶ��ٶȱ���
	/*

	�����ƶ��ٶȱ�����ֵ������ĺ���

	0                            No Movement Information Available
	1                            Aircraft Stopped (Ground Speed = 0 knots)
	2                            0 knots < Ground Speed �� 0.2315 km/h (0.125 kt)
	3 - 8                        0.2315 km/h (0.125 kt) < Ground Speed �� 1.852 km/h (1 kt) 0.2700833 km/h steps
	9 - 12                       1.852 km/h (1 kt) < Ground Speed �� 3.704 km/h (2 kt) 0.463 km /h (0.25 kt) steps
	13 - 38                      3.704 km/h (2 kt) < Ground Speed �� 27.78 km/h (15 kt) 0.926 km/h (0.50 kt) steps
	39 - 93                      27.78 km/h (15 kt) < Ground Speed �� 129.64 km/h (70 kt) 1.852 km/h (1.00 kt) steps
	94 - 108                     129.64 km/h (70 kt) < Ground Speed �� 185.2 km/h (100 kt) 3.704 km/h (2.00 kt) steps
	109 - 123                    185.2 km/h (100 kt) < Ground Speed �� 324.1 km/h (175 kt) 9.26 km/h (5.00 kt) steps
	124                          324.1 km/h (175 kt) < Ground Speed
	125                          Reserved for Aircraft Decelerating
	126                          Reserved for Aircraft Accelerating
	127                          Reserved for Aircraft Backing-Up
	*/
	BYTE HeadingState ;//��λ״̬( 0--��λ������ 1--��λ����)
	DOUBLE Heading ;   //��λ(��λ:�Ƕ�)
}SurfacePosInfo , *PSurfacePosInfo;
//_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-���صķָ���
/*
DF=17 , TypeCode=19
��������:Airborne Velocity Message
*/

//��������������ٶ���Ϣ�ṹ
typedef struct TagVelocityInfo
{


	BYTE SubType ;         //������ (GS/AS)
	//1 = GS/Normal (�ٶ���1000������)
	//2 = GS/Supersonic (�ٶ���1022������)
	//3 = Airspeed/Heading/Normal (�����ٷ�����)
	//4 = Airspeed/Heading/Supersonic (����Ϊ���ٳ���1000�ڵķ�����)

	BOOL Supersonic ;                     //�����ٱ�־(GS/AS)
	BYTE IntentChangeFlag ;               //Intent Change Flag (0--��־û�иı� 1--��־�ı�)(GS/AS)
	BYTE NACv ;                           //���о��ȼ���(GS/AS)
	BYTE Dire_w ;                         //���ٶ�������(0--�� 1--����)(GS)
	UINT Velocitye_w ;					 //�����ٶ�(��λ:�� kts)(GS)
	BYTE Dirn_s;                          //�����ϱ�����(0--�� 1--����)(GS)
	UINT  Velocityn_s;					 //�ϱ��ٶ�(��λ:�� kts)(GS)
	DOUBLE GS ;                           //����(��λ:�� kts)(GS)
	DOUBLE GSHeading ;                    //���ٷ���(GS)
	BOOL HdgAvailStatusBit ;              //����״̬���(false--���ٺ�����Ϣ������ true--���ٺ�����Ϣ����)(AS)
	DOUBLE ASHeading;                      //���ٷ���(AS)
	BOOL TASFlag ;                        //true--����Ϊ����� ��false--����Ϊָʾ����(AS)
	INT IAS ;                             //ָʾ����(��λ:kts)(AS)
	INT TAS;                              //�����(��λ:�� kts)(AS)
	BYTE VRateSourceBit ;                 //Source Bit For Vertical Rate( 0--Geometric 1--Baro)(GS/AS)
	BYTE CurState ;                       //����״̬(0-����   1-�½�)(GS/AS)
	INT VRate ;                           //����(��λ: Ӣ��/���� ft/min) ,��ֵΪ��������ֵΪ�½�(GS/AS)
	INT GPSAltDiff ;                      //GPS�߶�����ѹ�߶ȵ�����ֵ(��ֵ˵��GPS�߶ȴ�����ѹ�߶ȣ���ֵΪ������ѹ�߶�)(GS/AS)    

	BOOL GSInfoAvailable ;                //���������Ϣ�Ƿ����
	BOOL ASInfoAvailable ;                //���������Ϣ�Ƿ����


}VelocityInfo,*PVelocityInfo;
//////////////////////////////////////////////////////////////////////////���صķָ���
typedef struct TagOperationalStatusInfo
{
BYTE HDR ; //ˮƽ���ο�����( 1--�ų���  0--����)
}OperationalStatusInfo ; 
//_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-���صķָ���
//��������������Ϣ
typedef struct AircraftInfo
{
	ICAOAddress                ICAOID ;                     //ICAO��ַ
	IdentificationInfo         IdAndCategory;              //���������ʶ��
	AircraftLocationInfo       Location ;                  //λ����Ϣ
	SurfacePosInfo             SurfaceState;               //�����λ����ص���Ϣ
	VelocityInfo               VelocityState;              //�����ٶ������Ϣ
	OperationalStatusInfo      StatusInfo ;                //����״̬��Ϣ
	ADS_Time                   LocationLastUpdateTime ;    //λ����Ϣ���ĸ���ʱ��
	ADS_Time                   InfoLastUpdateTime ;        //�յ����ڴ˷����������ݵ����ʱ��
	BOOL                       bTimeOut;                   //������Ŀ�곬ʱ(��ʱ��δ�յ���Ŀ����κ���Ϣ)
	unsigned int			   squawk;						//Ӧ������
	AltitudeReplayInfo		   AltitudeReplay;				//�߶�Ӧ��ظ�
}AircraftInfo,*PAircraftInfo ; 





typedef void* LPVOID ; 
typedef LPVOID DE_HANDLE ; 
typedef void ( * pfnVisitCallBack)(const AircraftInfo* );
#define INVALID_DECODE_HANDLE  0

//////////////////////////////////////////////////////////////////////////���صķָ���

/*
�����µĽ�����ʵ��
����ֵ:������ʵ���ľ��,ʧ�ܷ���INVALID_DECODE_HANDLE
*/
DE_HANDLE InitNewDecoder(double home_lon , double home_lat ) ; 

/*
�������������ص�����λ����Ϣ
*/
void SetDecoderHome(DE_HANDLE deHandle ,double lon , double lat) ;

/*
����DF=17��ADS-B��չ��Ϣ
����ֵ������ɹ������λ����Ϣ����TRUE�� ���򷵻�FALSE
*/
BOOL DecodeADSMessage(DE_HANDLE deHandle ,  // [IN] ���������
					  BYTE AdsMessage[14], // [IN] ADS��Ϣ����
					  ADS_Time Time       // [IN] ���Ľ���ʱ��
					  ) ;

/*
�������λ����Ϣ
����ֵ������ɹ������λ����Ϣ����TRUE�� ���򷵻�FALSE
*/
BOOL DecodeAirPostion(DE_HANDLE deHandle ,  // [IN] ���������
					  BYTE AdsMessage[14], // [IN] ADS��Ϣ����
					  ADS_Time Time       // [IN] ���Ľ���ʱ��
					  ) ;


/*
�������λ����Ϣ
����ֵ������ɹ������λ����Ϣ����TRUE�� ���򷵻�FALSE
*/
BOOL  DecodeSurfacePostion(DE_HANDLE deHandle ,  // [IN] ���������
						   BYTE AdsMessage[14], // [IN] ADS��Ϣ����
						   ADS_Time Time       // [IN] ���Ľ���ʱ��
						   ) ;

/*
�����ٶ���Ϣ
����ֵ:�������ɹ�����TRUE�����򷵻�FALSE
*/
BOOL DecodeVelocity(DE_HANDLE deHandle ,  // [IN] ���������
					BYTE AdsMessage[14],    // [IN] ADS��Ϣ����
					ADS_Time Time          // [IN] ���Ľ���ʱ��
					);

/*
�����ɻ�ʶ������������Ϣ
����ֵ:��
*/
BOOL DecodeIdAndCategoryInfo(DE_HANDLE deHandle ,  // [IN] ���������
							 BYTE AdsMessage[14],  // [IN] ADS����Ϣ����
							 ADS_Time Time         // [IN] ���Ľ���ʱ��
							 );
/*
���ߺ���
�����ɻ���ϸ����(���������ַ����ĳ���)
����ֵ:�����ַ����ĳ���,��������ڴ�����򷵻�NULL
*/
const char* GetAircraftCategoryDescription(
	BYTE CategorySet ,  // [IN] �����������(A,B,C,D)
	BYTE Category      // [IN] ��������ϸ���
	);

/*
�����ɻ���״̬��Ϣ
����ֵ:�������ɹ�����TRUE�����򷵻�FALSE
*/
BOOL DecodeOperationalStatus(DE_HANDLE deHandle ,  // [IN] ���������
							 BYTE AdsMessage[14],  // [IN] 112λADS����Ϣ����
							 ADS_Time Time         // [IN] ���Ľ���ʱ��
							 ) ; 
/*
�������ڽ��н����Ŀ������
*/
size_t GetTotalCount(DE_HANDLE deHandle)  ; 

/*
����ָ����ICAO��Ŀ��������Ϣ
*/
AircraftInfo* GetAircraftInfo(DE_HANDLE deHandle , ICAOAddress ICAO);

/*
��鳬��ָ��ʱ���û���κθ��µ�Ŀ��
����ֵ:��ʱ��Ŀ�������
*/
size_t CheckTimeOut(DE_HANDLE deHandle,// [IN] ���������
					std::vector<ICAOAddress>& TimeOutAircrafts, //[OUT] �洢��ʱ��Ŀ���ICAO
					ADS_Time	RefTime ,  //  [IN] ���㳬ʱ�Ĳο�ʱ���
					int TimeOutSeconds,    //  [IN] ��ʱʱ������λ:�룩
					BOOL bSign             //  [IN] �Ƿ��ǳ�ʱ��Ŀ�꣬�����ǣ���ɾ����ʱĿ������Ὣ��ɾ��
					);

/*
�ڷ�������Ϣ���ݱ���ɾ�����г�ʱ����Ŀ , �ڵ���ǰ�ȵ���CheckTimeOut������鳬ʱ��Ϣ��Ŀ
����ֵ��ɾ������Ŀ������
*/
size_t DeleteTimeOut(DE_HANDLE deHandle// [IN] ���������
					 ) ; 
size_t DeleteTimeOutFromRefTime(DE_HANDLE deHandle , ADS_Time     RefTime ,  int TimeOutSeconds  ) ;

/*
���ʽ�������������Ϣ
ע:���ʺ���Ӧ�����ܶ�С ����ΪVisitAll�����ڲ�ʹ�ö�д�����糤ʱ��ռ�û���������̳߳�ʱ��ȴ�
*/
void VisitAll(DE_HANDLE deHandle , // [IN] ���������
			 pfnVisitCallBack fn  // [IN] ��������
			 ) ;


double real_lon(double lon ,double home_lon)  ; 
double real_lat(double lat , double home_lat)  ;
#endif
