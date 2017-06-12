#define  ADSB_DECODER_NO_LOCK
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "ADS_B_Decode.h"
#include <map>
#include <math.h>
#include <cassert>
#include "cpr.h"
#include "ads_altitude.h"
#include "crc.h"
#include "utility.h"

#ifndef ADSB_DECODER_NO_LOCK
typedef boost::shared_mutex rwmutex ; 
typedef boost::unique_lock<boost::shared_mutex> wLock ; 
typedef boost::shared_lock<boost::shared_mutex> rLock ; 

#define Write_Lock(p) wLock wl(*p) ; 
#define Read_Lock(p)  rLock rl(*p) ; 

#else

typedef void rwmutex ; 
#define Write_Lock(p)	(p) ; 
#define Read_Lock(p)  (p) ; 
#endif

typedef std::map<ICAOAddress,AircraftInfo>  DBType ; 
typedef DBType* LPDBType ; 	
typedef DBType::iterator DBPointer ;
typedef DBType::const_iterator ConstDBPointer ;

#define H2P(n) static_cast<LPDBType>(n)
#define MODES_LONG_MSG_BYTES     14
#define MODES_SHORT_MSG_BYTES    7
#define MODES_LONG_MSG_BITS     (MODES_LONG_MSG_BYTES    * 8)
#define MODES_SHORT_MSG_BITS    (MODES_SHORT_MSG_BYTES   * 8)

typedef unsigned int uint32_t ; 

typedef  struct decoder_contex
{
	rwmutex* mutex ;		//读写互斥量
	cpr_decoder* decoder ;  //CPR解码器
}decoder_contex ;

std::map<DE_HANDLE,decoder_contex> DecoderContex ; 
BOOL IsExistICAO(DE_HANDLE deHandle , ICAOAddress ICAO) ; 


ads_cpr_decode_param get_ads_cpr_param(BYTE ads_msg[14])
{
	ads_cpr_decode_param param ; 

	DWORD XZ_Value ;
	BYTE * pXZ_Data = &ads_msg[8];
	BYTE * pXZ_Value = (BYTE*)&XZ_Value;
	pXZ_Value[3] = pXZ_Data[0];
	pXZ_Value[2] = pXZ_Data[1];
	pXZ_Value[1] = pXZ_Data[2];
	pXZ_Value[0] = pXZ_Data[3];


	DWORD YZ_Value ;
	BYTE * pYZ_Data = &ads_msg[6];
	BYTE * pYZ_Value = (BYTE*)&YZ_Value;
	pYZ_Value[3] = pYZ_Data[0];
	pYZ_Value[2] = pYZ_Data[1];
	pYZ_Value[1] = pYZ_Data[2];
	pYZ_Value[0] = pYZ_Data[3];

	param.yz =(YZ_Value>>9)&0x0001FFFF;	//YZ(纬度Bin)
	param.xz = (XZ_Value>>8)&0x0001FFFF; //XZ(经度Bin)
	param.cpr_type = (ads_msg[6]>>2)&0x01; //奇偶标志

	return param ; 

}

rwmutex* GetRWMutex(DE_HANDLE deHandle)
{
#ifdef ADSB_DECODER_NO_LOCK
	return NULL ; 
#else
	std::map<DE_HANDLE,decoder_contex> ::iterator it = DecoderContex.find(deHandle) ;
	if(it==DecoderContex.end())
		return NULL ;
	else 
		return it->second.mutex; 
#endif

}

void SetDecoderHome(DE_HANDLE deHandle ,double lon , double lat)
{
	std::map<DE_HANDLE,decoder_contex> ::iterator it = DecoderContex.find(deHandle) ;
	if(it==DecoderContex.end())
		return  ;
	else 
	{
		cpr_loc_t loc = {lon , lat } ; 
		it->second.decoder->set_location(loc) ;
	}
}



int modesMessageLenByType(int type) {
	return (type & 0x10) ? MODES_LONG_MSG_BITS : MODES_SHORT_MSG_BITS;
}


static inline   unsigned getbit(unsigned char *data, unsigned bitnum)
{
	unsigned bi = bitnum - 1;
	unsigned by = bi >> 3;
	unsigned mask = 1 << (7 - (bi & 7));

	return (data[by] & mask) != 0;
}

// Extract some bits (firstbit .. lastbit inclusive) from a message.
static inline   unsigned getbits(unsigned char *data, unsigned firstbit, unsigned lastbit)
{
	unsigned fbi = firstbit - 1;
	unsigned lbi = lastbit - 1;
	unsigned nbi = (lastbit - firstbit + 1);

	unsigned fby = fbi >> 3;
	unsigned lby = lbi >> 3;
	unsigned nby = (lby - fby) + 1;

	unsigned shift = 7 - (lbi & 7);
	unsigned topmask = 0xFF >> (fbi & 7);

	assert(fbi <= lbi);
	assert(nbi <= 32);
	assert(nby <= 5);

	if (nby == 5) {
		return
			((data[fby] & topmask) << (32 - shift)) |
			(data[fby + 1] << (24 - shift)) |
			(data[fby + 2] << (16 - shift)) |
			(data[fby + 3] << (8 - shift)) |
			(data[fby + 4] >> shift);
	}
	else if (nby == 4) {
		return
			((data[fby] & topmask) << (24 - shift)) |
			(data[fby + 1] << (16 - shift)) |
			(data[fby + 2] << (8 - shift)) |
			(data[fby + 3] >> shift);
	}
	else if (nby == 3) {
		return
			((data[fby] & topmask) << (16 - shift)) |
			(data[fby + 1] << (8 - shift)) |
			(data[fby + 2] >> shift);
	}
	else if (nby == 2) {
		return
			((data[fby] & topmask) << (8 - shift)) |
			(data[fby + 1] >> shift);
	}
	else if (nby == 1) {
		return
			(data[fby] & topmask) >> shift;
	}
	else {
		return 0;
	}
}
int icaoFilterTest(uint32_t addr, DE_HANDLE deHandle)
{

	rwmutex* mutex = GetRWMutex(deHandle);
	Write_Lock(mutex); //写锁定

	LPDBType pDB = H2P(deHandle);
	ICAOAddress  ICAO = ICAOAddress((addr >> 16) & 0x000000FF, (addr >> 8) & 0x000000FF, addr & 0x000000FF);

	BOOL isExist = IsExistICAO(deHandle, ICAO); 
	return  isExist ? 1 : 0; 
}


#define INVALID_ALTITUDE (-9999)

int decodeID13Field(int ID13Field) {
	int hexGillham = 0;

	if (ID13Field & 0x1000) { hexGillham |= 0x0010; } // Bit 12 = C1
	if (ID13Field & 0x0800) { hexGillham |= 0x1000; } // Bit 11 = A1
	if (ID13Field & 0x0400) { hexGillham |= 0x0020; } // Bit 10 = C2
	if (ID13Field & 0x0200) { hexGillham |= 0x2000; } // Bit  9 = A2
	if (ID13Field & 0x0100) { hexGillham |= 0x0040; } // Bit  8 = C4
	if (ID13Field & 0x0080) { hexGillham |= 0x4000; } // Bit  7 = A4
													  //if (ID13Field & 0x0040) {hexGillham |= 0x0800;} // Bit  6 = X  or M 
	if (ID13Field & 0x0020) { hexGillham |= 0x0100; } // Bit  5 = B1 
	if (ID13Field & 0x0010) { hexGillham |= 0x0001; } // Bit  4 = D1 or Q
	if (ID13Field & 0x0008) { hexGillham |= 0x0200; } // Bit  3 = B2
	if (ID13Field & 0x0004) { hexGillham |= 0x0002; } // Bit  2 = D2
	if (ID13Field & 0x0002) { hexGillham |= 0x0400; } // Bit  1 = B4
	if (ID13Field & 0x0001) { hexGillham |= 0x0004; } // Bit  0 = D4

	return (hexGillham);
}

int ModeAToModeC(unsigned int ModeA)
{
	unsigned int FiveHundreds = 0;
	unsigned int OneHundreds = 0;

	if ((ModeA & 0xFFFF8889) != 0 ||         // check zero bits are zero, D1 set is illegal
		(ModeA & 0x000000F0) == 0) { // C1,,C4 cannot be Zero
		return INVALID_ALTITUDE;
	}

	if (ModeA & 0x0010) { OneHundreds ^= 0x007; } // C1
	if (ModeA & 0x0020) { OneHundreds ^= 0x003; } // C2
	if (ModeA & 0x0040) { OneHundreds ^= 0x001; } // C4

												  // Remove 7s from OneHundreds (Make 7->5, snd 5->7). 
	if ((OneHundreds & 5) == 5) { OneHundreds ^= 2; }

	// Check for invalid codes, only 1 to 5 are valid 
	if (OneHundreds > 5) {
		return INVALID_ALTITUDE;
	}

	//if (ModeA & 0x0001) {FiveHundreds ^= 0x1FF;} // D1 never used for altitude
	if (ModeA & 0x0002) { FiveHundreds ^= 0x0FF; } // D2
	if (ModeA & 0x0004) { FiveHundreds ^= 0x07F; } // D4

	if (ModeA & 0x1000) { FiveHundreds ^= 0x03F; } // A1
	if (ModeA & 0x2000) { FiveHundreds ^= 0x01F; } // A2
	if (ModeA & 0x4000) { FiveHundreds ^= 0x00F; } // A4

	if (ModeA & 0x0100) { FiveHundreds ^= 0x007; } // B1 
	if (ModeA & 0x0200) { FiveHundreds ^= 0x003; } // B2
	if (ModeA & 0x0400) { FiveHundreds ^= 0x001; } // B4

												   // Correct order of OneHundreds. 
	if (FiveHundreds & 1) { OneHundreds = 6 - OneHundreds; }

	return ((FiveHundreds * 5) + OneHundreds - 13);
}

typedef enum {
	UNIT_FEET,
	UNIT_METERS
} altitude_unit_t;

static int decodeAC13Field(int AC13Field, altitude_unit_t *unit) {
	int m_bit = AC13Field & 0x0040; // set = meters, clear = feet
	int q_bit = AC13Field & 0x0010; // set = 25 ft encoding, clear = Gillham Mode C encoding

	if (!m_bit) {
		*unit = UNIT_FEET;
		if (q_bit) {
			// N is the 11 bit integer resulting from the removal of bit Q and M
			int n = ((AC13Field & 0x1F80) >> 2) |
				((AC13Field & 0x0020) >> 1) |
				(AC13Field & 0x000F);
			// The final altitude is resulting number multiplied by 25, minus 1000.
			return ((n * 25) - 1000);
		}
		else {
			// N is an 11 bit Gillham coded altitude
			int n = ModeAToModeC(decodeID13Field(AC13Field));
			if (n < -12) {
				return INVALID_ALTITUDE;
			}

			return (100 * n);
		}
	}
	else {
		*unit = UNIT_METERS;
		// TODO: Implement altitude when meter unit is selected
		return INVALID_ALTITUDE;
	}
}

BOOL  DecodeAllReplay(DE_HANDLE deHandle,  // [IN] 解码器句柄
	BYTE msg[14], // [IN] ADS消息报文
	ADS_Time Time       // [IN] 报文接收时间
)
{
	unsigned int msgtype = getbits(msg, 1, 5); // Downlink Format
	unsigned int msgbits = modesMessageLenByType(msgtype);
	unsigned int crc = modesChecksum(msg, msgbits);

	int addr = 0 ;
	unsigned int IID = crc & 0x7f;
	if (crc & 0xffff80) {
		
		struct errorinfo *ei = modesChecksumDiagnose(crc & 0xffff80, msgbits);
		if (!ei) {
			return -2; // couldn't fix it
		}

		// see crc.c comments: we do not attempt to fix
		// more than single-bit errors, as two-bit
		// errors are ambiguous in DF11.
		if (ei->errors > 1)
			return -2; // can't correct errors

	int 	correctedbits = ei->errors;
	if (correctedbits == 0)
		modesChecksumFix(msg, ei);

		// check whether the corrected message looks sensible
		// we are conservative here: only accept corrected messages that
		// match an existing aircraft.
		addr = getbits(msg, 9, 32);
		if (!icaoFilterTest(addr , deHandle)) {
			return -1;
		}
	}

	if (IID == 0)
	{
		addr = getbits(msg, 9, 32);
		icaoFilterAdd(deHandle , Time	 , addr);

	//	printf("************************************************   %06X\r\n" , addr );
	}


	return TRUE;

}

BOOL DecodeAltitudeReplay(DE_HANDLE deHandle,  // [IN] 解码器句柄
	BYTE msg[14], // [IN] ADS消息报文
	ADS_Time Time       // [IN] 报文接收时间
)
{
	unsigned int msgtype = getbits(msg, 1, 5); // Downlink Format
unsigned int msgbits = modesMessageLenByType(msgtype);
unsigned int crc = modesChecksum(msg,msgbits);

if (!icaoFilterTest(crc, deHandle)) {
	return FALSE;
}

unsigned int addr = crc; //ICAO 

unsigned int AC = getbits(msg, 20, 32);
altitude_unit_t altitude_unit = UNIT_FEET;
int altitude_valid = 0;
int altitude = 0;

if (AC) { // Only attempt to decode if a valid (non zero) altitude is present
	altitude = decodeAC13Field(AC, &altitude_unit);
	if (altitude != INVALID_ALTITUDE)
		altitude_valid = 1;


	LPDBType pDB = H2P(deHandle);
	ICAOAddress  ICAO = ICAOAddress((addr >> 16) & 0x000000FF, (addr >> 8) & 0x000000FF, addr & 0x000000FF);
	DBPointer IT = pDB->find(ICAO);
	if (IT == pDB->end())
		return FALSE;

	AircraftInfo* InfoPointer = &(IT->second);


	InfoPointer->AltitudeReplay.altitude_valid = altitude_valid;
	InfoPointer->AltitudeReplay.altitude = altitude;
	InfoPointer->AltitudeReplay.altitude_unit = altitude_unit == UNIT_FEET ? 0 : 1;

	//printf("############   %06X   %d\r\n", addr, altitude);
	return TRUE;
}
else
return FALSE;

}



BOOL  DecodeSquawk(DE_HANDLE deHandle,  // [IN] 解码器句柄
	BYTE msg[14], // [IN] ADS消息报文
	ADS_Time Time       // [IN] 报文接收时间
)
{
	unsigned int msgtype = getbits(msg, 1, 5); // Downlink Format
	unsigned int msgbits = modesMessageLenByType(msgtype);
	unsigned int crc = modesChecksum(msg,msgbits);


	if (!icaoFilterTest(crc , deHandle)) {
		return FALSE ;
	}

	unsigned int addr = crc; //ICAO 

	LPDBType pDB = H2P(deHandle);
	ICAOAddress  ICAO = ICAOAddress((addr >> 16) & 0x000000FF, (addr >> 8) & 0x000000FF, addr & 0x000000FF);
	DBPointer IT = pDB->find(ICAO);
	if (IT == pDB->end())
		return FALSE ; 

	AircraftInfo* InfoPointer = &(IT->second);

	//int ID13Field = ((msg[2] <<8) | msg[3]) & 0x1FFF;
	unsigned int ID = getbits(msg, 20, 32);
	if (ID) {
		unsigned int squawk = decodeID13Field(ID);

		InfoPointer->squawk = squawk;

		unsigned int identity = 0;
		{
			int a, b, c, d;

			a = ((msg[3] & 0x80) >> 5) |
				((msg[2] & 0x02) >> 0) |
				((msg[2] & 0x08) >> 3);
			b = ((msg[3] & 0x02) << 1) |
				((msg[3] & 0x08) >> 2) |
				((msg[3] & 0x20) >> 5);
			c = ((msg[2] & 0x01) << 2) |
				((msg[2] & 0x04) >> 1) |
				((msg[2] & 0x10) >> 4);
			d = ((msg[3] & 0x01) << 2) |
				((msg[3] & 0x04) >> 1) |
				((msg[3] & 0x10) >> 4);
			identity = a * 1000 + b * 100 + c * 10 + d;
		}
		return TRUE;
	}
	else
		return FALSE; 
}



DE_HANDLE InitNewDecoder(double home_lon , double home_lat)
{
	DBType* pNewInstance = new DBType ;
	assert(pNewInstance);

	if(pNewInstance==NULL)
		return INVALID_DECODE_HANDLE ; 

	decoder_contex contex ; 

	cpr_loc_t home_location = {home_lon , home_lat} ; 

#ifdef ADSB_DECODER_NO_LOCK
	contex.mutex  = NULL ; 
#else
	contex.mutex = new rwmutex ; 
#endif

	contex.decoder = new cpr_decoder(home_location);
	assert(contex.decoder);

	if( contex.decoder==NULL)
		return INVALID_DECODE_HANDLE ; 

	DecoderContex.insert(std::pair<DE_HANDLE,decoder_contex>( static_cast<DE_HANDLE>(pNewInstance) , contex)) ; 
	return static_cast<DE_HANDLE>(pNewInstance);
}


cpr_decoder* get_decoder(DE_HANDLE deHandle) 
{
	std::map<DE_HANDLE,decoder_contex> ::iterator it = DecoderContex.find(deHandle) ;
	if(it==DecoderContex.end())
		return NULL ;
	else 
		return it->second.decoder; 
}



BOOL IsExistICAO(DE_HANDLE deHandle , ICAOAddress ICAO)
{
	LPDBType pDB = H2P(deHandle) ; 

	ConstDBPointer  p = pDB->find(ICAO);
	return (p!=pDB->end())?TRUE:FALSE ; 
}


//初始化数据库条目
void InitAircraftEntry(DE_HANDLE deHandle ,ICAOAddress ICAO , ADS_Time Time , AircraftInfo* pInfo)
{
	pInfo->ICAOID = ICAO ; 

	//对IdentificationInfo结构进行初始化
	memset(pInfo->IdAndCategory.FlightID,0,8) ; 
	memset(pInfo->IdAndCategory.OriFlightID , 0 ,6);
	pInfo->IdAndCategory.CategorySet = CategorySetA ; 
	pInfo->IdAndCategory.Category = 0 ; 

	//对AircraftLocationInfo结构进行初始化
	pInfo->Location.Altitude     = 0 ;
	pInfo->Location.EvalLon      = 0 ; 
	pInfo->Location.EvalLat      = 0 ;
	pInfo->Location.LocType      = Air ; 
	pInfo->Location.LocPercision.NACP = UNKNOWN_NACP ; 
	pInfo->Location.LocPercision.PercisionVer = VERSION_ZERO ; 
	pInfo->Location.LocPercision.NIC = NULL_PERCISION ; 
	pInfo->Location.LocPercision.NUC = NULL_PERCISION ; 
	pInfo->Location.SurveillanceStatus = 0 ;
	pInfo->Location.T = INVALID_T ; 
	pInfo->Location.Q = INVALID_Q ; 
	pInfo->Location.LocationAvailable = FALSE ; 

	//对SurfacePosInfo结构进行初始化
	pInfo->SurfaceState.MoveMentCode = 0 ;
	pInfo->SurfaceState.Movement = 0 ;
	pInfo->SurfaceState.HeadingState = 0 ;
	pInfo->SurfaceState.Heading=  0 ; 

	//对VelocityInfo结构进行初始化
	pInfo->VelocityState.GSInfoAvailable = FALSE ; 
	pInfo->VelocityState.ASInfoAvailable = FALSE ; 

	//设置最近更新时间
	pInfo->LocationLastUpdateTime = Time ;

	pInfo->bTimeOut = FALSE ; 

}
//增加新的条目
void AddEntry(DE_HANDLE deHandle , ICAOAddress ICAO ,const AircraftInfo& Info)
{
	LPDBType pDB = H2P(deHandle) ; 
	pDB->insert(std::pair<ICAOAddress,AircraftInfo>(ICAO , Info)) ; 
}
void  icaoFilterAdd(DE_HANDLE deHandle, ADS_Time Time, uint32_t addr)
{
	LPDBType pDB = H2P(deHandle);
	ICAOAddress  ICAO = ICAOAddress((addr >> 16) & 0x000000FF, (addr >> 8) & 0x000000FF, addr & 0x000000FF);
	DBPointer IT = pDB->find(ICAO);
	if (IT != pDB->end())
		return;

	AircraftInfo Info;
	InitAircraftEntry(deHandle, ICAO, Time, &Info);
	AddEntry(deHandle, ICAO, Info);

}

//返回指定ICAO目标的信息
AircraftInfo* GetAircraftInfo(DE_HANDLE deHandle , ICAOAddress ICAO)
{
	LPDBType pDB = H2P(deHandle) ; 

	rwmutex* mutex = GetRWMutex(deHandle)  ;
	Read_Lock(mutex) ; //读锁定

	DBPointer  p = pDB->find(ICAO);
	if(p==pDB->end()) return NULL ; 
	return &(p->second) ; 
}


//返回当前所有目标的数量
size_t GetTotalCount(DE_HANDLE deHandle) 
{
	LPDBType pDB = H2P(deHandle) ; 

	rwmutex* mutex = GetRWMutex(deHandle)  ;
	Read_Lock(mutex) ; //读锁定
	return pDB->size() ; 
}

/////////////////////////////////////工具函数/////////////////////////////////////朴素的分割线
time_t  MakeADSTime(ADS_Time ads_time)
{
	time_t t_of_day=mktime(&ads_time.t);
	return t_of_day ;  
}

//////////////////////////空中位置相关解码////////////////////////////////////////////////朴素的分割线

//进行全局解码时两条信息最大的时间间隔(10秒)
#define VALID_POSITION_UPDATE_INTERVAL 60

typedef unsigned long ULONG_PTR, *PULONG_PTR;
typedef ULONG_PTR DWORD_PTR, *PDWORD_PTR;
#ifndef MAKEWORD
#define MAKEWORD(a, b)      ((WORD)(((BYTE)((DWORD_PTR)(a) & 0xff)) | ((WORD)((BYTE)((DWORD_PTR)(b) & 0xff))) << 8))
#endif

//当VER = 0 时获取其NUC
DWORD GetNUC(const BYTE* Data)
{
	BYTE Code_Type = 	(Data[4]>>3)&0x1F; //消息类型
	DWORD a = (DWORD)Code_Type ; 
	return DWORD(18-a) ; 
}


//当VER = 1 时获取其NIC
DWORD GetNIC(const  BYTE* Data ,BYTE NIC_SUPPLEMENT_A)
{
	static DWORD onzero[10] = {11,10,8,7,6,5,4,2,1,0} ; 
	static DWORD onone[10] =  {11,10,9,7,6,5,4,3,1,0} ; 
	BYTE Code_Type = 	(Data[4]>>3)&0x1F; //消息类型

	if(NIC_SUPPLEMENT_A==0)
	{		return onzero[Code_Type-9] ; 
	}
	else
	{
		return onone[Code_Type-9] ; 
	}
}


const double  AirDlat0   = 6.0;  
const double  AirDlat1   = 360.0 / 59.0;
const double  SurfDlat0  = 1.5;
const double  SurfDlat1 = 90.0 / 59.0;




bool float_val_equ(double a ,double b)
{
	const double min_flt= 1e-3 ;
	return fabs(a-b)<min_flt;

}

BOOL DecodeAirPostion(DE_HANDLE deHandle ,  // [IN] 解码器句柄
	BYTE AdsMessage[14], // [IN] ADS消息报文
	ADS_Time Time       // [IN] 报文接收时间
	) 
{
	rwmutex* mutex = GetRWMutex(deHandle)  ;
	Write_Lock(mutex) ; //写锁定

	BOOL bHavePos = FALSE ; 
	LPDBType pDB = H2P(deHandle) ; 
	ICAOAddress  ICAO = ICAOAddress(AdsMessage[1] ,AdsMessage[2] ,AdsMessage[3]) ;

	if(!IsExistICAO(deHandle , ICAO)) //数据库中不包含此飞行器条目
	{
		AircraftInfo Info ; 
		InitAircraftEntry(deHandle , ICAO,Time,&Info) ; 
		AddEntry(deHandle , ICAO ,Info ) ;
	}

	DBPointer IT = pDB->find(ICAO) ; 
	AircraftInfo* InfoPointer = &(IT->second) ; 

	InfoPointer->InfoLastUpdateTime = Time ;  //设置最新更新时间
	if(InfoPointer->bTimeOut)
	{
		InfoPointer->bTimeOut = FALSE ; 
	}

	BYTE Code_Type = 	(AdsMessage[4]>>3)&0x1F; //DF17消息类型


	if(  (Code_Type>=9 && Code_Type<=18 ) /*||( Code_Type>=20 &&Code_Type<=22)*/  ) //包含空中经纬度信息
	{

		BYTE CPRType = (AdsMessage[6]>>2)&0x01; //奇偶标志
		DWORD Ver = InfoPointer->Location.LocPercision.PercisionVer ; 
		//解析时间同步标志位T
		InfoPointer->Location.T =  (AdsMessage[6]>>3)&0x01 ; 

		if(Code_Type>=9 && Code_Type<=18 )		//解释精度级别
		{
			InfoPointer->Location.AltType = Baro ; 

			InfoPointer->Location.LocPercision.NIC_SUPPLEMENT_B = AdsMessage[4]&0x01 ;  //NIC_Supplement_B
			if(Ver == VERSION_ONE)
			{
				DWORD NIC = GetNIC(AdsMessage ,InfoPointer->Location.LocPercision.NIC_SUPPLEMENT_A) ; 
				InfoPointer->Location.LocPercision.NIC = NIC ; 
			}
			else if(Ver == VERSION_ZERO)
			{
				DWORD NUC = GetNUC(AdsMessage) ; 
				InfoPointer->Location.LocPercision.NUC = NUC ; 
			}		

		}else
		{
			InfoPointer->Location.AltType = HAE	 ; 
			if(Ver == VERSION_ONE)
			{
				if(Code_Type==20){InfoPointer->Location.LocPercision.NIC = 11 ; }
				else if(Code_Type==21){InfoPointer->Location.LocPercision.NIC = 10 ; }
				else {InfoPointer->Location.LocPercision.NIC = 0 ; }
			}
			else if(Ver == VERSION_ZERO)
			{
				if(Code_Type==20){InfoPointer->Location.LocPercision.NUC = 9 ; }
				else if(Code_Type==21){InfoPointer->Location.LocPercision.NUC =8 ; }
				else {InfoPointer->Location.LocPercision.NUC = 0 ; }
			}		

		}

		InfoPointer->Location.LocType = Air ;  //空中目标
		//解码高度信息
		unsigned short  alt_word = get_ads_b_alt_word(AdsMessage) ;
		int real_alt =  decode_alt(alt_word , 0) ; 
		InfoPointer->Location.Altitude = real_alt ;


		//解码位置信息
		cpr_decoder* decoder = get_decoder(deHandle);
		assert(decoder);

		ads_cpr_decode_param cpr_param  = get_ads_cpr_param(AdsMessage) ;

		cpr_loc_t loc =   decoder->decode((DWORD)ICAO ,cpr_param.yz ,cpr_param.xz ,cpr_param.cpr_type ,0) ; 
		if(!valid_location(loc))
		{
			bHavePos = FALSE ;
			InfoPointer->Location.LocationAvailable = FALSE  ;
		}
		else
		{
			//如果之前的位置时可用的，则测试当前解码的位置，和之前解码的位置的差值是否很大
			if(InfoPointer->Location.LocationAvailable) 
			{
				if( (fabs(InfoPointer->Location.EvalLon  - loc.lon  )>2 )   \
					|| (fabs(InfoPointer->Location.EvalLat -  loc.lat) >2) )
				{


					bHavePos = FALSE ;
					InfoPointer->Location.LocationAvailable = FALSE  ;

					decoder->reset((DWORD)ICAO) ;
				}
				else
				{
					InfoPointer->Location.EvalLon =  loc.lon;
					InfoPointer->Location.EvalLat = loc.lat;

					bHavePos = TRUE ;
					InfoPointer->Location.LocationAvailable = TRUE  ;

				}
			}
			else
			{

				InfoPointer->Location.EvalLon =  loc.lon;
				InfoPointer->Location.EvalLat = loc.lat;

				bHavePos = TRUE ;
				InfoPointer->Location.LocationAvailable = TRUE  ;
			}
		}
	}
	InfoPointer->LocationLastUpdateTime =  Time;
	return bHavePos ;

}

/////////////////////////////////速度相关信息解码/////////////////////////////////////////朴素的分割线
#define PI 3.1415926
const double DEGREE = 180.0/PI;
//计算地面速度的航向
double TrueTrack(int Velocityn_s,int Velocitye_w,int Signlat,int signlon)
{
	double Track = 0.0;
	if(Velocityn_s==-1 && Velocitye_w==-1) return 0 ;

	if(Velocityn_s==0)
	{
		if(signlon==0)
			return 90.0;
		else
			return 270.0;

	}

	Track = DEGREE*atan((double)Velocitye_w/((double)Velocityn_s));
	if(Signlat==1)
	{
		if(signlon==0)
			Track = 180-Track;
		else
			Track = 180+Track;
	}
	else
	{

		if(signlon==1)
			Track = 360 - Track;
	}

	return Track ;

}
BOOL DecodeVelocity(DE_HANDLE deHandle ,     // [IN] 解码器句柄
	BYTE AdsMessage[14],    // [IN] ADS消息报文
	ADS_Time Time          // [IN] 报文接收时间
	)
{

	rwmutex* mutex = GetRWMutex(deHandle)  ;
	Write_Lock(mutex) ; //写锁定

	LPDBType pDB = H2P(deHandle) ; 

	ICAOAddress  ICAO = ICAOAddress(AdsMessage[1] ,AdsMessage[2] ,AdsMessage[3]) ;
	if(!IsExistICAO(deHandle , ICAO)) //数据库中不包含此飞行器条目
	{
		AircraftInfo Info ; 
		InitAircraftEntry(deHandle , ICAO,Time,&Info) ; 
		AddEntry(deHandle , ICAO ,Info ) ;
	}
	DBPointer IT = pDB->find(ICAO) ; 
	AircraftInfo* InfoPointer = &(IT->second) ; 


	InfoPointer->InfoLastUpdateTime = Time ;  //设置最新更新时间
	if(InfoPointer->bTimeOut)
	{
		InfoPointer->bTimeOut = FALSE ; 
	}

	BYTE Code_Type = 	(AdsMessage[4]>>3)&0x1F; //消息类型	
	if(Code_Type!=19) return FALSE ; //消息类型不是19的，不进行处理


	BYTE Payload[8];
	memset(Payload,0,8);
	memcpy(&Payload[1],&AdsMessage[4],7); 
	InfoPointer->VelocityState.Velocitye_w = -1; //设置初始值
	InfoPointer->VelocityState.Velocityn_s = -1;
	InfoPointer->VelocityState.ASHeading = -1 ;

	BYTE SubType  = Payload[1]&0x07 ;//析出子类型 
	InfoPointer->VelocityState.SubType = SubType;  
	InfoPointer->VelocityState.Supersonic = ((Payload[1]&0x01)^0x01)?true:false; //是否为超音速类型的飞行器
	InfoPointer->VelocityState.IntentChangeFlag = (Payload[2]&0x80)>>7; //Intent Change Flag (0--标志没有改变 1--标志改变)
	InfoPointer->VelocityState.NACv = (Payload[2]&0x38)>>3;                    //导航精度级别
	switch(SubType)
	{

	case 1:
	case 2: 
		{
			InfoPointer->VelocityState.GSInfoAvailable = true ; 


			InfoPointer->VelocityState.Dire_w = (Payload[2]&0x04)>>2	; //东西方向标志
			if(SubType==1)
			{
				InfoPointer->VelocityState.Velocitye_w = ((Payload[2]&0x03)<<8)+Payload[3]-1;
			}
			else
			{
				InfoPointer->VelocityState.Velocitye_w = (((Payload[2]&0x03)<<8)+Payload[3]-1)*4;
			}

			InfoPointer->VelocityState.Dirn_s =(Payload[4]&0x80)>>7;    //南北方向标志
			if(SubType==1)
			{
				InfoPointer->VelocityState.Velocityn_s = ((((Payload[4]&0x7F)<<8) + ((Payload[5]&0xE0)))>>5)-1;
			}
			else
			{
				InfoPointer->VelocityState.Velocityn_s = (((((Payload[4]&0x7F)<<8) + ((Payload[5]&0xE0)))>>5)-1)*4;
			}

			//计算地面速度航向
			InfoPointer->VelocityState.GSHeading = TrueTrack(InfoPointer->VelocityState.Velocityn_s,InfoPointer->VelocityState.Velocitye_w,InfoPointer->VelocityState.Dirn_s,InfoPointer->VelocityState.Dire_w);

			if(InfoPointer->VelocityState.Velocityn_s==-1 && InfoPointer->VelocityState.Velocitye_w == -1)
			{
				InfoPointer->VelocityState.GS = 0;
			}	
			else
			{
				InfoPointer->VelocityState.GS  = (sqrt((double)(InfoPointer->VelocityState.Velocityn_s*InfoPointer->VelocityState.Velocityn_s+InfoPointer->VelocityState.Velocitye_w*InfoPointer->VelocityState.Velocitye_w)));
			}

		}
		break;
	case 3:
	case 4: 
		{
			InfoPointer->VelocityState.ASInfoAvailable = true ; 

			InfoPointer->VelocityState.HdgAvailStatusBit =((Payload[2]&0x04)>>2)?true:false; //空中航向是否可用
			if(InfoPointer->VelocityState.HdgAvailStatusBit) //可以解算航向
			{
				InfoPointer->VelocityState.ASHeading =(((Payload[2]&0x03)<<8) + Payload[3])*360.0/1024.0 ; 

			}
			InfoPointer->VelocityState.TASFlag = ((Payload[4]&0x80)>>7)?true:false;	 //使用IAS或者TAS标志
			if(InfoPointer->VelocityState.TASFlag)  
			{
				if(SubType==3)
				{
					InfoPointer->VelocityState.TAS = (  ( ((Payload[4]&0x7F)<<8)   +   ((Payload[5]&0xE0)) ) >>5   )-1 ;//真空速
				}
				else
				{
					InfoPointer->VelocityState.TAS =( (  ( ((Payload[4]&0x7F)<<8)   +   ((Payload[5]&0xE0)) ) >>5   )-1 )*4;//真空速
				}
			}
			else
			{
				if(SubType==3)
				{
					InfoPointer->VelocityState.IAS = (  ( ((Payload[4]&0x7F)<<8)   +   ((Payload[5]&0xE0)) ) >>5   )-1;//指示空速
				}
				else
				{
					InfoPointer->VelocityState.IAS = ((  ( ((Payload[4]&0x7F)<<8)   +   ((Payload[5]&0xE0)) ) >>5   )-1)*4;//指示空速
				}
			}

		}
		break;
	}

	InfoPointer->VelocityState.VRateSourceBit = (Payload[5]&0x10)>>4 ; //Source Bit For Vertical Rate
	BYTE c = (Payload[5]&0x08)>>3;
	InfoPointer->VelocityState.CurState = c;//飞行状态( 0--爬升 1--下降)

	int i =  (((Payload[5]&0x07)<<8) +(Payload[6]&0xFC))>>2;
	// 	if(i>256)
	// 		i = 9900;
	// 	else
	i=(i-1)*64;

	InfoPointer->VelocityState.VRate = i;
	if(c==1)            //下降状态，速度为负值
		InfoPointer->VelocityState.VRate = InfoPointer->VelocityState.VRate*(-1);


	c = (Payload[7]&0x80)>>7;
	if((Payload[7]&0x7F) ==0)
		InfoPointer->VelocityState.GPSAltDiff=0;
	else
		InfoPointer->VelocityState.GPSAltDiff = ((Payload[7]&0x7F)-1)*25;

	if(c==1)
		InfoPointer->VelocityState.GPSAltDiff = InfoPointer->VelocityState.GPSAltDiff*(-1);
	return TRUE ;


}

///////////////////////////////////ID相关信息解码///////////////////////////////////////朴素的分割线
const char CharLookup[] =  " ABCDEFGHIJKLMNOPQRSTUVWXYZ                     0123456789     ";

const char* CategoryDescriptions[4][8] = 
{
	{"No ADS-B Emitter Category Information" , 
	"Light (< 15500 lbs)" , 
	"Small (15500 to 75000 lbs)",
	"Large (75000 to 300000 lbs)",
	"High Vortex Large (aircraft such as B-757)",
	"Heavy (> 300000 lbs)",
	"High Performance (> 5g acceleration and 400 kts)",
	"Rotorcraft"},

	{"No ADS-B Emitter Category Information",
	"Glider / sailplane",
	"Lighter-than-air",
	"Parachutist / Skydiver",
	"Ultralight / hang-glider / paraglider",
	"Reserved",
	"Unmanned Aerial Vehicle",
	"Space / Trans-atmospheric vehicle"},

	{"No ADS-B Emitter Category Information",
	"Surface Vehicle – Emergency Vehicle",
	"Surface Vehicle – Service Vehicle",
	"Point Obstacle (includes tethered balloons)",
	"Cluster Obstacle",
	"Line Obstacle",
	"Reserved",
	"Reserved"},

	{"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved",
	"Reserved"
	"Reserved",
	"Reserved"}

} ;

BYTE Convert2Byte(BYTE* pByte,int Index)//注意:Index基数为1
{

	if(Index%2==1) //奇数
	{
		return pByte[Index/2];
	}
	else //偶数
	{
		BYTE Ret = 0;
		BYTE a = pByte[Index/2-1]; 
		BYTE b = pByte[Index/2];

		BYTE t1 = a&0x0F;
		Ret = Ret|t1;
		Ret = Ret<<4;
		BYTE t2 = b&0xF0;
		t2 =t2>>4;
		Ret = Ret|t2;
		return Ret;
	}
}
BOOL DecodeIdAndCategoryInfo(DE_HANDLE deHandle ,  // [IN] 解码器句柄
	BYTE AdsMessage[14],  // [IN] ADS长消息报文
	ADS_Time Time         // [IN] 报文接收时间
	)
{
	rwmutex* mutex = GetRWMutex(deHandle)  ;
	Write_Lock(mutex) ; //写锁定


	LPDBType pDB = H2P(deHandle) ; 

	ICAOAddress  ICAO = ICAOAddress(AdsMessage[1] ,AdsMessage[2] ,AdsMessage[3]) ;
	if(!IsExistICAO(deHandle , ICAO)) //数据库中不包含此飞行器条目
	{
		AircraftInfo Info ; 
		InitAircraftEntry(deHandle , ICAO,Time,&Info) ; 
		AddEntry(deHandle , ICAO ,Info ) ;
	}
	DBPointer IT = pDB->find(ICAO) ; 
	AircraftInfo* InfoPointer = &(IT->second) ; 

	InfoPointer->InfoLastUpdateTime = Time ;  //设置最新更新时间
	if(InfoPointer->bTimeOut)
	{
		InfoPointer->bTimeOut = FALSE ; 
	}
	BYTE Code_Type = 	(AdsMessage[4]>>3)&0x1F; //消息类型	
	if(!(Code_Type>=1 && Code_Type<=4)) return FALSE ; 

	//复制原始的6字节的ID代码
	memcpy(InfoPointer->IdAndCategory.OriFlightID ,&AdsMessage[5] , 6 );

	memset(InfoPointer->IdAndCategory.FlightID,0,8);
	InfoPointer->IdAndCategory.FlightID[0] = CharLookup[Convert2Byte(AdsMessage,11)>>2];
	InfoPointer->IdAndCategory.FlightID[1] = CharLookup[Convert2Byte(AdsMessage,12)%64];
	InfoPointer->IdAndCategory.FlightID[2] = CharLookup[Convert2Byte(AdsMessage,14)>>2];
	InfoPointer->IdAndCategory.FlightID[3] = CharLookup[Convert2Byte(AdsMessage,15)%64];
	InfoPointer->IdAndCategory.FlightID[4] = CharLookup[Convert2Byte(AdsMessage,17)>>2];
	InfoPointer->IdAndCategory.FlightID[5] = CharLookup[Convert2Byte(AdsMessage,18)%64];
	InfoPointer->IdAndCategory.FlightID[6] = CharLookup[Convert2Byte(AdsMessage,20)>>2];
	InfoPointer->IdAndCategory.FlightID[7] = CharLookup[Convert2Byte(AdsMessage,21)%64];

	Code_Type = 	(AdsMessage[4]>>3)&0x1F;
	InfoPointer->IdAndCategory.CategorySet = Code_Type ; 

	InfoPointer->IdAndCategory.Category = AdsMessage[4]&0x07 ; 
	return TRUE ; 
}



const char* GetAircraftCategoryDescription(
	BYTE CategorySet ,  // [IN] 飞行器总类别(A,B,C,D)
	BYTE Category      // [IN] 飞行器详细类别
	)
{
	if(CategorySet>4 || CategorySet<1) return  NULL ;
	if(Category>7) return NULL ;

	return CategoryDescriptions[4-CategorySet][Category] ; 
}

/////////////////////////////////飞机状态相关/////////////////////////////////////////朴素的分割线
BOOL DecodeOperationalStatus(DE_HANDLE deHandle ,  // [IN] 解码器句柄
	BYTE AdsMessage[14],  // [IN] 112位ADS长消息报文
	ADS_Time Time         // [IN] 报文接收时间
	) 
{
	rwmutex* mutex = GetRWMutex(deHandle)  ;
	Write_Lock(mutex) ; //写锁定


	LPDBType pDB = H2P(deHandle) ; 

	ICAOAddress  ICAO = ICAOAddress(AdsMessage[1] ,AdsMessage[2] ,AdsMessage[3]) ;
	if(!IsExistICAO(deHandle , ICAO)) //数据库中不包含此飞行器条目
	{
		AircraftInfo Info ; 
		InitAircraftEntry(deHandle , ICAO,Time,&Info) ; 
		AddEntry(deHandle , ICAO ,Info ) ;
	}
	DBPointer IT = pDB->find(ICAO) ; 
	AircraftInfo* InfoPointer = &(IT->second) ; 
	InfoPointer->InfoLastUpdateTime = Time ;  //设置最新更新时间
	if(InfoPointer->bTimeOut)
	{
		InfoPointer->bTimeOut = FALSE ; 
	}

	BYTE Code_Type = 	(AdsMessage[4]>>3)&0x1F; //消息类型	
	if(Code_Type!=31) return FALSE ; 

	//目前只解析其精度版本信息
	BYTE Per = (AdsMessage[9]>>5)&0x07 ;
	if(Per==0)
	{
		InfoPointer->Location.LocPercision.PercisionVer = VERSION_ZERO ; 
	}else if(Per==1)
	{
		InfoPointer->Location.LocPercision.PercisionVer = VERSION_ONE ; 
	}
	else
	{

		InfoPointer->Location.LocPercision.PercisionVer = VERSION_ZERO ; 
	}
	InfoPointer->Location.LocPercision.NIC_SUPPLEMENT_A = (AdsMessage[9]>>4)&0x01 ;  //VERSION ==1 时NIC补充位

	InfoPointer->Location.LocPercision.NACP = AdsMessage[9]&0x0F ;  //位置导航经度级别
	InfoPointer->StatusInfo.HDR = (AdsMessage[10]>>2)&0x01 ;        //水平方参考方向

	return TRUE  ;

}

//////////////////////////////////////////////////////////////////////////朴素的分割线
size_t CheckTimeOut(DE_HANDLE deHandle,    // [IN] 解码器句柄
	std::vector<ICAOAddress>& TimeOutAircrafts, //[OUT] 存储超时的目标的ICAO
	ADS_Time	RefTime ,  //  [IN] 计算超时的参考时间点
	int TimeOutSeconds,    //  [IN] 超时时长（单位:秒）
	BOOL bSign             //  [IN] 是否标记超时的目标，如果标记，则删除超时目标操作会将其删除
	)
{

	rwmutex* mutex = GetRWMutex(deHandle)  ;
	Write_Lock(mutex) ; //写锁定

	LPDBType pDB = H2P(deHandle) ; 

	if(pDB->empty()) return 0  ; 

	size_t Index = 0 ; 

	DBPointer p = pDB->begin() ; 
	for( ; p!=pDB->end() ; p++)
	{
		time_t T0 = MakeADSTime(p->second.InfoLastUpdateTime) ; 
		time_t T1 = MakeADSTime(RefTime) ; 
		double TimeDiff = difftime(T1, T0) ; 
		int  SecondInterval = static_cast<int>(TimeDiff) ; 

		if(SecondInterval>TimeOutSeconds) 
		{
			if(bSign) p->second.bTimeOut = TRUE; 
			TimeOutAircrafts.push_back(p->second.ICAOID) ; 
			Index++ ; 
		}

	}
	return Index ; 
}
size_t DeleteTimeOut(DE_HANDLE deHandle// [IN] 解码器句柄
	) 
{
	rwmutex* mutex = GetRWMutex(deHandle)  ;
	Write_Lock(mutex) ; //写锁定

	LPDBType pDB = H2P(deHandle) ; 

	if(pDB->empty()) return 0  ; 
	size_t Count = 0 ; 
	DBPointer p = pDB->begin() ; 
	for(;p!=pDB->end() ; )
	{
		if(p->second.bTimeOut)
		{
			pDB->erase(p++) ;
			Count++ ; 
		}
		else
		{
			++p ; 
		}
	}
	return Count ; 
}

size_t DeleteTimeOutFromRefTime(DE_HANDLE deHandle , ADS_Time     RefTime ,  int TimeOutSeconds  )
{

	LPDBType pDB = H2P(deHandle) ;
	if(pDB->empty()) return 0  ;
	size_t Count = 0 ;

	DBPointer p = pDB->begin() ;
	for(   ; p!=pDB->end(); )
	{
		time_t T0 = MakeADSTime(p->second.InfoLastUpdateTime) ;
		time_t T1 = MakeADSTime(RefTime) ;
		double TimeDiff = difftime(T1, T0) ;
		int  SecondInterval = static_cast<int>(TimeDiff) ;

		if(SecondInterval>TimeOutSeconds)
		{
			pDB->erase(p++) ;
			Count++ ;
		}
		else
		{
			++p ;
		}

	}
	return Count ;
}




void VisitAll(DE_HANDLE deHandle,// [IN] 解码器句柄
	pfnVisitCallBack fn// [IN] 遍历函数
	)
{
	rwmutex* mutex = GetRWMutex(deHandle)  ;
	Read_Lock(mutex) ; //读锁定

	LPDBType pDB = H2P(deHandle) ; 
	DBPointer p = pDB->begin() ; 
	for(;p!=pDB->end() ;p++ )
	{
		fn(&(p->second)) ; 
	}

}

////////////////////////////////地面位置相关//////////////////////////////////////////朴素的分割线
//获取地面移动速度信息 
double GetMovement(BYTE MovementByte)
{
	if(MovementByte == 0)
	{
		return 0 ;
	}else if(MovementByte == 1)
	{
		return  0 ;
	}else if(MovementByte == 2)
	{
		return 0.2315 ; 
	}else if(MovementByte>=3 && MovementByte<=8)
	{
		return (MovementByte-3+1)*0.2700833+0.2315 ; 
	}else if(MovementByte>=9 && MovementByte<=12)
	{
		return (MovementByte-9+1)*0.463+1.852 ; 
	}else if(MovementByte>=13 && MovementByte<=38)
	{
		return (MovementByte-13+1)*0.926+3.704 ;
	}else if(MovementByte>=39 && MovementByte<=93)
	{
		return (MovementByte-39+1)*1.852+27.78 ;
	}else if(MovementByte>=94 && MovementByte<=108)
	{
		return (MovementByte-94+1)*3.704+129.64 ;
	}else if(MovementByte>=109 && MovementByte<=123)
	{
		return (MovementByte-109+1)*9.26+185.2 ;
	}else if(MovementByte == 124)
	{
		return 324.1 ; 
	}
	else
	{
		return 0 ;
	}
}


BOOL DecodeADSMessage(DE_HANDLE deHandle ,  // [IN] 解码器句柄
	BYTE AdsMessage[14], // [IN] ADS消息报文
	ADS_Time Time       // [IN] 报文接收时间
	)
{

	BYTE DF_Code = (AdsMessage[0]>>3)&0x1F; 

	bool isShort = false;
	if (AdsMessage[7] == 0 && AdsMessage[8] == 0 && AdsMessage[9] == 0)
		isShort = true;

	if (DF_Code == 4 || DF_Code == 20 && isShort)
	{
	//	return DecodeAltitudeReplay(deHandle, AdsMessage, Time);
		return FALSE ;
	}
	else if (DF_Code == 11 && isShort)
	{
	return   DecodeAllReplay(deHandle, AdsMessage, Time);
	}
	else if (DF_Code==5 || DF_Code==21 && isShort)
	{
	return  	DecodeSquawk(deHandle, AdsMessage, Time);
	}
	else if(DF_Code==17 && isShort ==false	){

		if (isShort)
			return FALSE; 

		DWORD a = CRC24(AdsMessage , 14);

		uint32_t crc = modesChecksum(AdsMessage ,112);
		if (crc)
			return FALSE; 

		BYTE Code_Type = (AdsMessage[4]>>3)&0x1F; //消息类型
		switch(Code_Type)
		{
		case 1:
		case 2:
		case 3:
		case 4:
			{

				return DecodeIdAndCategoryInfo(deHandle ,AdsMessage ,Time ) ; 
			}
			break ; 
		case 5:
		case 6:
		case 7:
		case 8:
			{
				return 	DecodeSurfacePostion(deHandle ,AdsMessage ,Time ) ; 
			}
			break ;
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
			{
				return DecodeAirPostion(deHandle ,AdsMessage ,Time ) ; 

			}
			break ; 
		case 19:
			{
				return 	DecodeVelocity(deHandle ,AdsMessage ,Time ) ; 
			}
			break;
		case 20:
		case 21:
		case 22:
			{
				return 	DecodeAirPostion(deHandle ,AdsMessage ,Time ) ; 
			}
			break ; 

		case 31:
			{
				return 	DecodeOperationalStatus(deHandle ,AdsMessage ,Time) ; 
			}
			break ; 

		default:
			return FALSE ; 
		}
	}
	else if( isShort==false && (DF_Code==18) &&( (AdsMessage[0]&0x01)==1 ))
	{
		return 	DecodeSurfacePostion(deHandle ,AdsMessage ,Time ) ; 
	}
	else
		return FALSE ;


}

double real_lat(double lat , double home_lat)
{

	return lat ;

	if(home_lat>0)
		return lat ; 
	else 
		return -90.0 + lat ; 

}


double real_lon(double lon ,double home_lon)
{
	return lon ;

	double diff = 0 ;
	if(lon>=0)
		diff = lon - int(lon/90.0)*90 ; 
	else
	{
		double a = -lon ; 

		diff = (int(a/90.0) +1) *90 -a  ; 
	}

	double rl =0 ; 
	if(home_lon>=0)
	{
		rl =  int(home_lon/90.0)*90.0 + diff ; 

	}
	else{

		rl =  (int(home_lon/90.0)-1)*90.0 + diff ; 

	}
	if(rl>=180)
		rl = rl - 180.0 ; 

	if(rl<=-180.0)
		rl = rl +180.0 ; 

	return rl ; 

}

//////////////////////////////////////////////////////////////////////////朴素的分割线
BOOL  DecodeSurfacePostion(DE_HANDLE deHandle ,  // [IN] 解码器句柄
	BYTE AdsMessage[14], // [IN] ADS消息报文
	ADS_Time Time       // [IN] 报文接收时间
	) 
{

	rwmutex* mutex = GetRWMutex(deHandle)  ;
	Write_Lock(mutex) ; //写锁定

	BOOL bHavePos =FALSE ; 
	BYTE Code_Type =  (AdsMessage[4]>>3)&0x1F; //消息类型	


	LPDBType pDB = H2P(deHandle) ; 
	ICAOAddress  ICAO = ICAOAddress(AdsMessage[1] ,AdsMessage[2] ,AdsMessage[3]) ;

	if(!IsExistICAO(deHandle , ICAO)) //数据库中不包含此飞行器条目
	{
		AircraftInfo Info ; 
		InitAircraftEntry(deHandle , ICAO,Time,&Info) ; 
		AddEntry(deHandle , ICAO ,Info ) ;
	}

	DBPointer IT = pDB->find(ICAO) ; 
	AircraftInfo* InfoPointer = &(IT->second) ; 

	InfoPointer->InfoLastUpdateTime = Time ;  //设置最新更新时间
	if(InfoPointer->bTimeOut)
	{
		InfoPointer->bTimeOut = FALSE ; 
	}

	if(Code_Type>=5 && Code_Type<=8) //包含地面经纬度信息
	{

		DWORD NIC = GetNIC(AdsMessage ,0) ; 
		InfoPointer->Location.LocPercision.NIC = NIC ;
		InfoPointer->Location.LocPercision.PercisionVer = VERSION_ONE ; 


		//移动速度信息
		BYTE MovementByte = ((AdsMessage[4]&0x07)<<4)|((AdsMessage[5]&0xF0)>>4) ;
		InfoPointer->SurfaceState.MoveMentCode = MovementByte ; 
		InfoPointer->SurfaceState.Movement = GetMovement(MovementByte) ; 


		//行驶方向信息
		BYTE headState = (AdsMessage[5]&0x08)>>3 ; 
		InfoPointer->SurfaceState.HeadingState = headState ; 
		if(headState!=0)
		{
			BYTE HeadCode = ((AdsMessage[5]&0x07)<<4)|((AdsMessage[6]&0xF0)>>4) ; 
			InfoPointer->SurfaceState.Heading = HeadCode*360.0/128.0 ; 
		}

		InfoPointer->Location.LocType = Surface ;  //地面目标
		//解析时间同步标志位T
		InfoPointer->Location.T =  (AdsMessage[6]>>3)&0x01 ; 

		cpr_decoder* decoder = get_decoder(deHandle);
		assert(decoder);

		ads_cpr_decode_param cpr_param  = get_ads_cpr_param(AdsMessage) ;

		cpr_loc_t loc =   decoder->decode((DWORD)ICAO ,cpr_param.yz ,cpr_param.xz ,cpr_param.cpr_type ,1) ; 

		if(!valid_location(loc))
		{
			bHavePos = FALSE ;
			InfoPointer->Location.LocationAvailable = FALSE  ;
		}
		else
		{
			//如果之前的位置时可用的，则测试当前解码的位置，和之前解码的位置的差值是否很大
			if(InfoPointer->Location.LocationAvailable) 
			{
				if( (fabs(InfoPointer->Location.EvalLon  - loc.lon  )>2 )   \
					|| (fabs(InfoPointer->Location.EvalLat -  loc.lat) >2) )
				{

					bHavePos = FALSE ;
					InfoPointer->Location.LocationAvailable = FALSE  ;

					decoder->reset((DWORD)ICAO) ;
				}
				else
				{
					InfoPointer->Location.EvalLon =  loc.lon;
					InfoPointer->Location.EvalLat = loc.lat;
					bHavePos = TRUE ;
					InfoPointer->Location.LocationAvailable = TRUE  ;
				}
			}
			else
			{
				InfoPointer->Location.EvalLon =  loc.lon;
				InfoPointer->Location.EvalLat = loc.lat;

				bHavePos = TRUE ;
				InfoPointer->Location.LocationAvailable = TRUE  ;
			}

		}
	}
	InfoPointer->LocationLastUpdateTime =  Time;
	return bHavePos ;
}
