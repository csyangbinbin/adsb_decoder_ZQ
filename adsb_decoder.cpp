//#define		ADSB_API_EXPORTS
#ifdef _WINDOWS
#include <Windows.h>
#endif
#include "adsb_decoder_api.h" 
#include "decoder_src/std_3h_dtatpack.h"
#include "decoder_src/adsb_core_decoder.h"
#include "decoder_src/DecodeResultProcess.h"
#include "debug.h"
#include <string.h>


#define  MAX_ADSB_MSG_BUFFER_LEN 512

enum fpga_data_type
{
	UNKNOWN_FPGA_DATA_TYPE	=	0, //δ֪����
	AIS_FPGA_DATA_TYPE ,  //AIS����
	ADS_FPGA_DATA_TYPE ,   //ADS����
	AC_FPGA_DATA_TYPE	//AC����
} ;

//ȫ�ֽ��������
ADSB_FPGA_DecoderInterface*		fpga_message_decoder = NULL   ;//FPGA���ݴ���
adsb_core_decoder*				adsb_decoder= NULL ;			//ADS-B������
ADSB_ResultProcess*				result_processer = NULL  ;		//��������ݴ���


//��鱨������
static fpga_data_type	check_data_type(   unsigned char* data ,size_t count) 
{
	if(data == NULL || count ==0 )
		return UNKNOWN_FPGA_DATA_TYPE ; 

	if(data[0] == 0x0D && data[1] == 0x0A && data[2] == 0xBE)
		return ADS_FPGA_DATA_TYPE ;
	else if(data[0] == 0x3B && data[1] ==0x03)
		return AC_FPGA_DATA_TYPE ;
	else
		return AIS_FPGA_DATA_TYPE ; 
}



#define  INIT_DECODE_RESULT(n) \
	adsb_decode_result_t n ; \
	memset((void*)&n , 0 ,sizeof(adsb_decode_result_t)); \
	n.decode_type = decode_error_type ;




static bool  init_all_decoder()
{
	//��ʼ��Ӳ�����ݽ�����
	if(fpga_message_decoder	==	NULL)
	{
		fpga_message_decoder  =  new std3h_fpga_decoder(); 
		if(fpga_message_decoder==NULL || !fpga_message_decoder->init())
		{
			log_print("FPGA���ݽ�������ʼ��ʧ��!\r\n");
			return  false; 
		}
	}

	//��ʼ��adsb_decoder������
	if(adsb_decoder ==NULL)
	{
		adsb_decoder  = new adsb_core_decoder();
		if(adsb_decoder == NULL || !adsb_decoder->init(121 , 37 ))
		{
			log_print("ads-b���ݽ�������ʼ��ʧ��");
			return false;
		}	
	}


	if(result_processer == NULL )
	{
		result_processer  = new ADSB_ResultProcess(adsb_decoder);
		if(result_processer == NULL || !result_processer->Init())
		{
			log_print("ADSB_ResultProcess init error");
			return false; 
		}

	}

	return true ; 
}



//����Ӳ���������
adsb_decode_result_t	adsb_decode(  char* data , size_t data_size) 
{
	INIT_DECODE_RESULT(result);

	if(data==NULL)
		return result ; 

	//��ʼ�����еĽ�����
	bool all_decoder_inited = init_all_decoder() ; 
	if(!all_decoder_inited)
		return result ;

	//������ݱ��ĵ�����
	fpga_data_type data_type = check_data_type((unsigned char*)data , data_size ) ; 
	if(data_type!= ADS_FPGA_DATA_TYPE)
	{
		log_print("���յ��������ʹ���!\r\n");
		return	result ; 
	}

	BYTE ADS_B_MSG[MAX_ADSB_MSG_BUFFER_LEN];
	memset(ADS_B_MSG , 0 , MAX_ADSB_MSG_BUFFER_LEN);

	ADS_Time ADS_B_Time ; 
	memset(&ADS_B_Time , 0 , sizeof(ADS_Time));

	extra_info ei ; 
	memset(&ei ,  0 ,sizeof(extra_info));
	ei.valid =			false ; 
	ei.gps_time_valid =	false;


	//�����ݱ��Ľ��н���
	int de_result = 	fpga_message_decoder->Decode((unsigned char*)data , data_size , ADS_B_MSG ,ADS_B_Time  ,&ei) ; 
	if(de_result == FPGA_DATA_MSG	)
	{
		//��ADS-B����
		DWORD adsb_msg_de_ret =adsb_decoder->decode( ADS_B_MSG, ADS_B_Time );
		return 	result_processer->ProcessResult(adsb_msg_de_ret , ADS_B_MSG, ADS_B_Time ,&ei);
	}
	else if ( de_result == FPGA_TIME_MSG)
	{
		//��Ӳ��״̬������Ϣ
		return result_processer->ProcessDevInfo(&ei);
	}
	else
	{
		return result ; 
	}


	return result ;
}

//ICAO��ַת��ΪUINT
static unsigned int icao_2_uint(adsb_icao_addr_t addr )  
{
	return	  ((((unsigned int)addr.m1)<<16)|(((unsigned int)addr.m2)<<8)|((unsigned int)addr.m3)) ; 
}


#if 0
int Sqlite_Open_For_Win(const char* dbname, sqlite3** db, int Flags, const char *zVfs)
{
	int nlen=0, codepage;
	char cpath[MAX_PATH];
	wchar_t wpath[MAX_PATH];
	codepage = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
	nlen = MultiByteToWideChar(codepage, 0, dbname, -1, NULL, 0);
	MultiByteToWideChar(CP_ACP, 0, dbname, -1, wpath, nlen); 
	nlen=WideCharToMultiByte(CP_UTF8 , 0, wpath, -1, 0, 0, 0, 0);
	WideCharToMultiByte(CP_UTF8 , 0, wpath, -1, cpath, nlen, 0, 0);
	return sqlite3_open_v2(cpath, db, Flags, zVfs);
}
#endif



#define CRC_CCITT 0x1021    //CRC-CCITT����ʽ
using namespace std;

unsigned int crc_ta_8[256]={ /* CRC �ֽ���ʽ�� */
	0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
	0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
	0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
	0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
	0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
	0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
	0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
	0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
	0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
	0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
	0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
	0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
	0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
	0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
	0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
	0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
	0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
	0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
	0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
	0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
	0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
	0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
	0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
	0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
	0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
	0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
	0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
	0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
	0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
	0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
	0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
	0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

unsigned int crc_ta_4[16]={ /* CRC ���ֽ���ʽ�� */
	0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
	0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
};
unsigned int crc_cal_by_halfbyte(unsigned char* ptr, unsigned char len)
{
	unsigned short crc = 0;

	while(len-- != 0)
	{
		unsigned char high = (unsigned char)(crc/4096); //�ݴ�CRC�ĸ�4λ
		crc <<= 4;
		crc ^= crc_ta_4[high^(*ptr/16)];

		high = (unsigned char)(crc/4096);
		crc <<= 4;
		crc ^= crc_ta_4[high^(*ptr&0x0f)];

		ptr++;
	}

	return crc;
}
#pragma  pack(push ,1)

typedef struct dev_net_config_pack_tag
{
	char			frame_head[3] ; //֡ͷ
	byte			version;		//�汾
	unsigned long 	self_ip_addr ;	//�豸IP��ַ
	word			self_port ;//�豸�˿ں�
	byte			self_mac[6];	//MAC��ַ
	unsigned long	dst_ip;	//Ŀ��IP
	word			dst_port;	//Ŀ��˿�
	byte			flag;		//��־λ
	byte			reverse[3];	//����
	word			crc;			//CRCУ��
} dev_net_config_pack; 

#pragma pack(pop)


 // char* str = "\x0D\x0A\xBE\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xC0\xA8\x08\x0E\x14\xB5\x18\x00\x00\x00\x11\x72";
