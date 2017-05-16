#ifndef __ADSB_DATA_PROTOCOL_INCLUDE__
#define __ADSB_DATA_PROTOCOL_INCLUDE__

typedef unsigned char u8;
typedef char s8;
typedef unsigned short u16;
typedef short s16;
typedef unsigned int u32;
typedef int s32;


/** @defgroup adsparm ADS-BĿ��״̬�ı���Ϣ����
*  ADS-BĿ��״̬�ı���Ϣ����
*  @{
*/



/*! \def ADS_B_ID_TAG
\brief ADS-BĿ��ID��Ϣ��ǩ
*/
#define ADS_B_ID_TAG        0xFF01
/*! \def ADS_B_LOC_TAG
\brief ADS-B����λ����Ϣ��ǩ
*/
#define ADS_B_LOC_TAG       0xFF02
/*! \def ADS_B_VEL_TAG
\brief ADS-B�ٶ���Ϣ��ǩ
*/
#define ADS_B_VEL_TAG       0xFF03
/*! \def ADS_B_OP_TAG
\brief ADS-B����״̬��Ϣ��ǩ
*/
#define ADS_B_OP_TAG        0xFF04
/*! \def ADS_B_SURFACE_TAG
\brief ADS-B������Ϣ��ǩ
*/
#define ADS_B_SURFACE_TAG   0xFF09
/*! \def ADS_B_SIGNAL_TAG
\brief ADS-B�ź���Ϣ��ǩ
*/
#define ADS_B_SIGNAL_TAG    0xFF05
/*! \def ADS_B_SIGNAL_TAG
\brief Ӳ�����ӶϿ���ǩ
*/
#define IS_HD_CONNECT_TAG   0xFF06

/*! \def ACARS_DATA_TAG
\brief ACARS���ݱ�ǩ
*/
#define ACARS_DATA_TAG		0xFF07

/*! \def ADS_B_TDAZIMUTHINFO_TAG
\brief ʱ�������Ϣ


*/
#define ADS_B_TDAZIMUTHINFO_TAG   0xFF10
/*! \def VERSION_ZERO
\brief ���ȼ���汾-0
*/

#define VERSION_ZERO      0

/*! \def VERSION_ONE
\brief ���ȼ���汾-1
*/
#define VERSION_ONE       1

/*! \def VERSION_NULL
\brief ���ȼ���汾δ֪
*/
#define VERSION_NULL      3

#pragma pack(1)

/*!
* @brief ������ʱ����ʱ���ʾ
*
* ������ʱ����ʱ���ʾ
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
* @brief ADS-BĿ��ICAO��ű�ʾ
*
* ADS-BĿ��ICAO��ű�ʾ
*/
typedef struct TagICAO
{
	u8 m1; ///< ��λ�ֽ�
	u8 m2; ///< һ�ֽ�
	u8 m3; //< ��λ�ֽ�
} ICAO;

/*!
* @brief ADS-B��Ϣͨ�ýṹ
*
* ADS-B��Ϣͨ�ýṹ
*/
typedef struct TagCommInfo
{
	u16 Tag;			///< �����ǩ
	ICAO Addr;			///< ICAO��ַ
	struct TimeInfo
	{
		struct TM t;	///< ʱ����Ϣ(������ʱ����)
		u32 tv_msec;	///< ����
		u32 tv_nsec;	///< ����
	} Timestamp;		///< ʱ���
	u8 ADSB_MSG[14];	 ///< ԭʼ��ADS-B������Ϣ
} CommInfo;


/*!
* @brief �����������ʶ����Ϣ
*
*	�����������ʶ����Ϣ
*/
typedef struct TagADSBIdentificationInfo
{
	CommInfo Comm_info;	///< ͨ����Ϣ
	s8 FlightID[8];		///< ID����
	u8 CategorySet;		///< ���
	u8 Category;		///< ���
} ADSBIdentificationInfo;


/*!
* @brief ADS-Bλ�þ�����Ϣ
*
*	ADS-Bλ�þ�����Ϣ
*/
typedef struct tagADSBPosPercision
{
	u8 Version;			///<���Ȱ汾
	union
	{
		u8 NUC_Value;	///< NUC
		struct
		{
			u8 NIC;		///<  NIC
			u8 NIC_SUPPLEMENT_A; ///< ����ֵA
			u8 NIC_SUPPLEMENT_B; ///< ����ֵB
		} NIC_Value;	///< NIC
	} Value;			///< λ�þ���ֵ
	u8 NACP;			///< λ�õ������ȼ���(��DF=17,TypeCode=31 ����Ϣ�д���)
} ADSBPosPercision;

/*!
* @brief ADS-Bλ����Ϣ
*
*	ADS-Bλ����Ϣ
*/
typedef struct TagADSBLocationInfo
{
	CommInfo Comm_info;				///< ͨ����Ϣ
	ADSBPosPercision PosPercision;	///< λ�þ�����Ϣ
	s32 Alt;						///< �߶�(��λ:M)
	s32 Lon;						///< ����(��λ:0.0000001�Ƕ�)
	s32 Lat;						///< γ��(��λ:0.0000001�Ƕ�)
	s32 GPSAltDiff;					///< GPS�߶�����ѹ�߶ȵ�����ֵ(��ֵ˵��GPS�߶ȴ�����ѹ�߶ȣ���ֵΪ������ѹ�߶� , ��λ:M)

} ADSBLocationInfo;

/*!
* @brief �ٶ���Ϣ����
*
*	�ٶ���Ϣ����
*/
typedef struct TagADSBVelocityInfoMisc
{
	u8 SubType;
	u8 NACv;
	u8 Dire_w;				 ///< ���ٶ�������(0--�� 1--����)(GS)
	u32 Velocitye_w;		 ///< �����ٶ�(��λ:�� kts)(GS)
	u8 Dirn_s;				 ///< �����ϱ�����(0--�� 1--����)(GS)
	u32 Velocityn_s;		 ///< �ϱ��ٶ�(��λ:�� kts)(GS)
	u8 VRateSourceBit;		 ///< Դ

} ADSBVelocityInfoMisc;

/*!
* @brief ADS-B�ٶ���Ϣ	
*
*	ADS-B�ٶ���Ϣ	
*/
typedef struct TagADSBVelocityInfo
{
	CommInfo Comm_info;				///< ͨ����Ϣ
	u32 GS;							///< ����(��λ:�� kts)
	u32 GSHeading;					///< ���ٷ���(��λ:0.01�Ƕ�) (0xFFFFFFFF��ʾ��Ч)
	s32 VRate;						///< ����(��λ: Ӣ��/���� ft/min) ,��ֵΪ��������ֵΪ�½�
	s32 GPSAltDiff;					///< GPS�߶�����ѹ�߶ȵ�����ֵ(��ֵ˵��GPS�߶ȴ�����ѹ�߶ȣ���ֵΪ������ѹ�߶� , ��λ:M)
	ADSBVelocityInfoMisc Misc ;		///< ������Ϣ
} ADSBVelocityInfo;

/*!
* @brief ADS-B������Ϣ
*
*	ADS-B������Ϣ	
*/
typedef struct TagADSSurfaceInfo
{
	CommInfo Comm_info;				///< ͨ����Ϣ
	ADSBPosPercision PosPercision ; ///< λ�þ�����Ϣ
	s32 Lon;						///< ����(��λ:0.0000001�Ƕ�)
	s32 Lat;						///< γ��(��λ:0.0000001�Ƕ�)
	u32 Movement ;					///< ���������ٶ�(��λ��0.01ǧ��/Сʱ km/h)
	u8  HeadingState ;				///< ��λ״̬( 0--��λ������ 1--��λ����)
	u32 Heading ;					///< ��λ(��λ:0.01 �Ƕ�)
} ADSSurfaceInfo;


/*!
* @brief ADS-B����״̬��Ϣ
*
*	ADS-B����״̬��Ϣ
*/
typedef struct TagADSBOperationalStatusInfo
{
	CommInfo Comm_info;		///< ͨ����Ϣ
} ADSBOperationalStatusInfo; ///< ״̬��Ϣ

/*!
* @brief ADS-B�ź������Ϣ
*
*	ADS-B�ź������Ϣ
*/
typedef struct TagSignalInfo
{
    CommInfo Comm_info;
    u8 snr ; //�����
    u8 pf[4] ; //Ƶ����Ϣ
} ADSBSignalInfo; 

/*!
* @brief Ӳ���Ƿ�������Ϣ
*
*	Ӳ���Ƿ�������Ϣ
*/
typedef struct TagNaNConnect  //Ӳ�����ӶϿ�
{
    u16 Tag;		// �����ǩ
    u8  conn_state ; //����״̬
} NaNConnect;

/*!
* @brief ˫վ�̻��߲�����Ϣ
*
*    ˫վ�̻��߲�����Ϣ
*/
typedef struct TagTDAzimuthInfo  //Ӳ�����ӶϿ�
{
    CommInfo Comm_info;
    double Azimuth1 ; //ʱ�����λֵ1(�Ƕ�)
    double Azimuth2 ; //ʱ�����λֵ2(�Ƕ�)
} TDAzimuthInfo;

/*!
* @brief ACARS������Ϣ
*
*	ACARS������Ϣ	
*/
typedef struct TagACARSInfo
{
    CommInfo Comm_info;				///< ͨ����Ϣ
    u8		 data[244];				///< ACARSԭʼ����
} ACARSInfo;

#pragma pack()


/** @} */ // end of adsparm


#endif