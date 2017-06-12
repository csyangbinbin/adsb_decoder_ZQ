#ifndef __ADSB_DECODER_API_INCLUDE__
#define __ADSB_DECODER_API_INCLUDE__


#include <map>
#include <vector>
#include <string>

#define ADSB_API   

#ifdef __cplusplus
extern "C"
{
#endif

	typedef unsigned char		u8;
	typedef char				s8;
	typedef unsigned short		u16;
	typedef short				s16;
	typedef unsigned int		u32;
	typedef int					s32;


	/** ���ջ�Ӳ�����͵�UDP�鲥���ݰ��ĳ��� */
#define ADSB_MESSAGE_PACKAGE_LEN			62


	/**
	* \brief ����ADS-B���ݱ��ĵ�ʱ�����Ϣ
	*/
	typedef struct adsb_timestamp_tag
	{
		u8	valid;		///<ʱ�����Ϣ�Ƿ���Ч�����δ��GPS/BDӲ������ʱ����Ч
		u32 tm_sec;		///< ��.	[0-60]  ,ע������ʱ���ΪUTCʱ��
		u32 tm_min;		///< ����.	[0-59] 
		u32 tm_hour;	///< Сʱ.	[0-23] 
		u32 tm_mday;	///< ��.	[1-31] 
		u32 tm_mon;		///< ��.	[1-12] 
		u32 tm_year;	///< ��.  
		u32 tv_msec;	///< ����
		u32 tv_wsec;	///< ΢��
		u32 tv_nsec;	///< ����
	}adsb_timestamp_t;


	/**
	* \brief ADS-BĿ��ICAO��ű�ʾ
	*
	*	ICAO��ַ�������ֽ����,����780A1F ,m1=0x78 , m2=0x0A ,m3=0x1F
	*/
	typedef struct adsb_icao_addr_tag
	{
		u8 m1;	///< ��λ�ֽ�
		u8 m2;	///< �м��ֽ�
		u8 m3;	///< ��λ�ֽ�
	} adsb_icao_addr_t;

	/**
	* \brief ADS-B��Ϣͨ�ýṹ
	*
	*/
	typedef struct adsb_common_info_tag
	{
		adsb_icao_addr_tag  icao_addr;		///< ICAO��ַ
		adsb_timestamp_t	timestamp;		///< ʱ���
		unsigned short		station_id;		///< ���ջ����
		unsigned int		squawk;			///< Ӧ������
	} adsb_common_info_t;

	/**
	* \brief ADS-B����λ����Ϣ
	*
	*/
	typedef struct adsb_air_location_info_tag
	{
		adsb_common_info_t	comm_info;		///< ͨ����Ϣ
		double	longitude;					///< ����(��λ:�Ƕ�)
		double	latitude;					///< γ��(��λ:�Ƕ�)
		double	altitude;					///< GPS�߶�(��λ:��)
	}adsb_air_location_info_t;

	/**
	* \brief ADS-B����λ����Ϣ
	*
	*/
	typedef struct adsb_surface_location_info_tag
	{
		adsb_common_info_t	comm_info;			///< ͨ����Ϣ
		double	longitude;						///< ����(��λ:�Ƕ�)
		double	latitude;						///< γ��(��λ:�Ƕ�)
		double	movement;						///< ���������ٶ�(��λ��ǧ��/Сʱ km/h)
		u8		heading_state;					///< ��ͷ������Ϣ�Ƿ����(0-������ , 1-����)
		double	heading_value;					///< ��ͷ����(��λ:�Ƕ�)
	}adsb_surface_location_info_t;


	/**
	* \brief �����������ʶ����Ϣ
	*
	*/
	typedef struct	adsb_identification_info_tag
	{
		adsb_common_info_t	comm_info;				///< ͨ����Ϣ
		s8 flight_id[8];							///< �����

		/**
		*	�����������Ϣ
		*
		*	1 = Aircraft identification, Category Set D	\n
		*	2 = Aircraft identification, Category Set C \n
		*	3 = Aircraft identification, Category Set B \n
		*	4 = Aircraft identification, Category Set A	\n
		*
		*/
		u8 category_set;

		/**
		* ADS-B������������ \n
		*		Set A	\n
		0 = No ADS-B Emitter Category Information	\n
		1 = Light (< 15500 lbs)	\n
		2 = Small (15500 to 75000 lbs)	\n
		3 = Large (75000 to 300000 lbs)	\n
		4 = High Vortex Large (aircraft such as B-757)	\n
		5 = Heavy (> 300000 lbs)	\n
		6 = High Performance (> 5g acceleration and 400 kts)	\n
		7 = Rotorcraft	\n
		
		Set B	\n
		 0 = No ADS-B Emitter Category Information	\n
		 1 = Glider / sailplane	\n
		2 = Lighter-than-air	\n
		3 = Parachutist / Skydiver	\n
		4 = Ultralight / hang-glider / paraglider	\n
		 5 = Reserved	\n
		6 = Unmanned Aerial Vehicle	\n
		7 = Space / Trans-atmospheric vehicle	\n
		
		 Set C	\n
		0 = No ADS-B Emitter Category Information	\n
		1 = Surface Vehicle �C Emergency Vehicle	\n
		2 = Surface Vehicle �C Service Vehicle	\n
		3 = Point Obstacle (includes tethered balloons)	\n
		4 = Cluster Obstacle	\n
		5 = Line Obstacle	\n
		6 = Reserved	\n
		7 = Reserved	\n

		Set D \n
		(Reserved)
		*/
		u8 category;
	} adsb_identification_info_t;


	/**
	* \brief ADS-B�ٶ���Ϣ
	*
	*/
	typedef struct	adsb_velocity_info_tag
	{
		adsb_common_info_t	comm_info;				///< ͨ����Ϣ
		double	gs_value;							///< ����(��λ:�� kts)
		double	gs_heading;							///< ���ٷ���(��λ:�Ƕ�) (��ֵ��־������Ϣ��Ч)
		s32		vrate;								///< ����(��λ: Ӣ��/���� ft/min) ,��ֵΪ��������ֵΪ�½�
	} adsb_velocity_info_t;


	/**
	*	\brief �豸�����Ϣ
	*
	* ��Щ�ͺŵ�ADS-B�����豸�а�װ�����أ��Ա㻧��û���е�������ʹ�á�
	* �����ʹ�õ��豸�в�������أ�����Դ˵����Ϣ
	*
	*/
	typedef struct dev_board_battery_info_tag
	{
		u8					state;				///< �����߷ŵ�(1--�ŵ� , 0--���)
		u8					capability;		///< ��ǰ�����İٷֱ�(ȡֵ:0 ~ 100)
		double				voltage;			///< ��ص�ǰ��ѹ
		double				current;			///< ��ص�ǰ����
		u16					work_time;			/**< ��ʾ���ʣ��ʱ�䣬����ش��ڷŵ�״̬ ,��ʾ���ʣ��Ĺ���ʱ�䣬
												����ش��ڳ��״̬��ʾ��س����绹��೤ʱ��	*/
		s8					battery_temperature;///< ����¶�(������ʾ���ϣ�������ʾ����)
	}dev_board_battery_info_t;

	/**
	*	\brief �豸�����ӵ�GPS/������λϵͳ��Ϣ
	*
	* ��Щ�ͺŵ�ADS-B�����豸�а�װ��GPS/������λģ�飬���ж�λ����ʱ�Ĺ��ܡ�
	* �����ʹ�õ��豸�в���GPS/�����豸������Դ���Ϣ
	*
	*/
	typedef struct dev_board_gps_info_tag
	{
		u8			satellite_count;		///< GPS/BD��������

		double		longitude;			///< ����(��λ:�Ƕ�) 
		double		latitude;			///< γ��(��λ:�Ƕ�)
		double		altitude;			///< �߶�(��λ:��)

		double		velocity_heading;		///< GPS/BD�ٶȷ���,������Ϊ�ο���׼(��λ:�Ƕ�)
		double		velocity_value;			///< GPS/BD�ٶȴ�С(��λ:�� knots)

		u32			year;					///< �� (UTC ʱ��)
		u32			month;					///< �� (UTC ʱ��)
		u32			day;					///< �� (UTC ʱ��)
		u32			hour;					///< ʱ (UTC ʱ��)
		u32			minute;					///< �� (UTC ʱ��)
		u32			second;				///< �� (UTC ʱ��)
	}dev_board_gps_info_t;

	/** ADS-Bͨ���������� */
#define ADSB_CHANNEL_STATE_WORKING		0

	/** ADS-Bͨ��δʹ�� */
#define ADSB_CHANNEL_STATE_UNUSED		1

	/** ADS-Bͨ������ */
#define ADSB_CHANNEL_STATE_FAULT		2

	/** ADS-Bͨ��״̬δ֪ */
#define ADSB_CHANNEL_STATE_UNDEFINED	3

	/**
	*	\brief �豸��·�ź�ͨ��״̬
	*
	*	��ADS-B�����豸������·�źŽ���ͨ����������ͨ����һ����������������״̬��
	*
	*/
	typedef struct dev_channel_state_info_tag
	{
		u8	channel_id;		///< ͨ��ID
		u8	channel_state;		///< ͨ��״̬
	} dev_channel_state_info_t;


	/**
	*	\brief ADS-B�����豸Ӳ��״̬��Ϣ
	*
	*	��ϵͳÿ���ӻᷢ��һ���豸��״̬��Ϣ�����а������豸���¶ȣ�GPS��Ϣ�������Ϣ��
	*	ͨ��״̬����Ӳ���汾�ŵ���Ϣ,�����ʱ��û�н��յ�����Ϣ������豸���ֹ��ϣ���������
	*	���ӳ������⡣
	*/
	typedef struct dev_board_info_tag
	{
		/**
		* ���ֽڵ���Ϣ��Ч��־λ����ʾ��Щ��Ϣ����Ч�ġ�
		*
		* �����ֽڵı�־λ���壬�Ӹ�λ����λ����Ϊ: \n
		* bit15 - �Ƿ�Ϊ�������� \n
		* bit14 - ���ջ������Ϣ�Ƿ���Ч \n
		* bit13 - GPS��Ϣ����������Ϣ�Ƿ���Ч \n
		* bit12 - GPS��Ϣ��ʱ������Ϣ�Ƿ���Ч \n
		* bit11 - GPS����������Ϣ�Ƿ���Ч \n
		* bit10 - GPS��γ����Ϣ�Ƿ���Ч \n
		* bit9 -  �ڲ�ʹ�����ݣ����� \n
		* bit8 -  �ڲ�ʹ�����ݣ����� \n
		* bit7 -  �ڲ�ʹ�����ݣ����� \n
		* bit6 -  �����Ϣ�Ƿ���Ч \n
		* bit5 -  �豸�¶���Ϣ�Ƿ���Ч \n
		* bit4 -  �źŴ���ͨ��״̬��Ϣ�Ƿ���Ч \n
		* bit3 -  �豸Ӳ����·�汾����Ϣ�Ƿ���Ч \n
		* bit2 -  �豸Ӳ���̼��汾����Ϣ�Ƿ���Ч \n
		* bit1 -  GPS�ٶ���Ϣ�Ƿ���Ч \n
		* bit0 -  �ڲ�ʹ�����ݣ����� \n
		*/
		u16							valid_flag;
		u16							station_id;			///< ���ջ����(ÿ�����ջ�Ψһ)
		dev_board_gps_info_t		gps_info;				///< GPS/BD��λ��Ϣ
		dev_board_battery_info_t	battery_info;			///< �����Ϣ	
		double						board_temperature;		///< �豸�¶�
		dev_channel_state_info_t	channel_state;			///< �źŴ���ͨ��״̬
		u8							dev_pcb_version;		///< �豸Ӳ����·�汾��
		u8							dev_firmware_version;	///< �豸Ӳ���̼��汾��
	}dev_board_info_t;


	/** �Ƿ�Ϊ������������ */
#define FLAG_TEST_DATA_MASK						(1<<15)

	/** ���ջ������Ϣ�Ƿ���Ч���� */
#define FLAG_STATION_ID_MASK					(1<<14)

	/** GPS��Ϣ����������Ϣ�Ƿ���Ч���� */
#define FLAG_GPS_YMD_MASK						(1<<13)

	/** ��Ϣ��ʱ������Ϣ�Ƿ���Ч���� */
#define FLAG_GPS_HMS_MASK						(1<<11)

	/** GPS����������Ϣ�Ƿ���Ч���� */
#define FLAG_GPS_SATELLITE_COUNT_MASK			(1<<11)

	/** GPS��γ����Ϣ�Ƿ���Ч���� */
#define FLAG_GPS_LOCATION_MASK					(1<<10)

	/** �����Ϣ�Ƿ���Ч���� */
#define FLAG_BATTERY_INFO_MASK					(1<<6)

	/** �豸�¶���Ϣ�Ƿ���Ч���� */
#define FLAG_BOARD_TEMP_MASK					(1<<5)

	/** �źŴ���ͨ��״̬��Ϣ�Ƿ���Ч���� */
#define FLAG_CHANNEL_STATE_MASK					(1<<4)

	/** �豸Ӳ����·�汾����Ϣ�Ƿ���Ч���� */
#define FLAG_PCB_VERSION_MASK					(1<<3)

	/** �豸Ӳ���̼��汾����Ϣ�Ƿ���Ч���� */
#define FLAG_FIRMWARE_VERSION_MASK				(1<<2)

	/** GPS�ٶ���Ϣ�Ƿ���Ч���� */
#define FLAG_GPS_VELOCITY_MASK					(1<<1)

	/**
	*	���Ľ���������
	*/
	enum adsb_decode_type
	{
		decode_error_type = 0,	///< ������޷�����ı���
		air_location_type,		///< ADS-B����λ����Ϣ����
		surface_location_type,	///< ADS-B����λ����Ϣ����
		identification_type,	///< ADS-B�������Ϣ����
		velocity_type,			///< ADS-B�ٶ���Ϣ����
		dev_board_info_type		///< �������豸״̬��Ϣ����
	};

	/**
	*	\brief ���Ľ�����
	*
	*/
	typedef struct adsb_decode_result_tag
	{
		adsb_decode_type					decode_type;					///< ����������
		union
		{
			adsb_air_location_info_t		air_location_info;				///< ADS-B����λ����Ϣ����
			adsb_surface_location_info_t	surface_location_info;			///< ADS-B����λ����Ϣ����
			adsb_identification_info_t		id_info;						///< ADS-B�������Ϣ����
			adsb_velocity_info_t			velocity_info;					///< ADS-B�ٶ���Ϣ����
			dev_board_info_t				board_info;					///< �������豸״̬��Ϣ����
		} decode_result;		///< ������
	}adsb_decode_result_t;

#define air_location_info_				decode_result.air_location_info
#define id_info_						decode_result.id_info
#define surface_location_info_			decode_result.surface_location_info
#define velocity_info_					decode_result.velocity_info
#define board_info_						decode_result.board_info


	/**
	*	\brief �����ݿ��в�ѯ��Ŀ��������ľ�̬��Ϣ���
	*
	*/
	typedef struct adsb_aircraft_information_tag
	{
		adsb_icao_addr_t icao;	///< ��������ICAO��ַ
		bool	query_result;	///< ���ݿ����Ƿ����ָ��ICAO��Ŀ�����Ϣ
		char registration[20];	///< ��������ע���
		char manufacturer[40]; ///< ��������������
		char model[5];			///< �������ͺż��,��B738
		char model_details[40];	///< �������ͺŵ���ϸ��Ϣ
		char country_iso[3];	///< �������������ҵ�ISO����,���й�CN,����US
	}adsb_aircraft_information_t;


	enum dev_config_enable_flag
	{
		dev_ip_addr_enable_flag = 0x80, //�豸�����IP��ַ���ø���
		dev_config_port_enable_flag = 0x40, //�豸��������˿����ø���
		dev_mac_addr_enable_flag = 0x20, //�豸�����MAC��ַ���ø���
		dest_ip_addr_enable_flag = 0x10, //����Ŀ�Ķ˵�IP��ַ���ø���
		dest_recv_port = 0x08,	//����Ŀ�Ķ˵Ľ��ն˿����ø���
		dev_config_all_enable = 0xF8 //�������ø���
	};

	/**
	*	\brief �豸�������ò���
	*
	*/
	typedef struct dev_net_config_tag
	{
		char				dev_ip_addr[16]; //�豸�����IP��ַ
		unsigned short		dev_config_port; //�豸��������˿�(�������ò�����PING����)
		unsigned char		dev_mac_addr[6]; //�豸�����MAC��ַ��6���ֽڣ�
		char				dest_ip_addr[16]; //����Ŀ�Ķ˵�IP��ַ
		unsigned short		dest_recv_port;  //����Ŀ�Ķ˵Ľ��ն˿�
		unsigned char		config_enable_flag;//���ò�����Ч��־λ
	}dev_net_config;


	typedef void* db_handle;

	/**
	* \brief ��������յ�ADS-B���ݱ��Ľ��н���
	*
	* \param [in] data	ָ����յı��ĵ��ڴ�ָ��.
	* \param [in] data_size ���յı��ĵĳ���.
	*
	* \note
	* ��ǰӲ�����ջ����͵ı��Ĳ���UDP�鲥Э�飬ÿ�����ĵĳ��ȹ̶�Ϊ62���ֽڡ�
	* ���ķ�Ϊ�������ͣ�\n 1.ADS-B���ݱ��� \n 2.Ӳ�����ջ�״̬��Ϣ���ģ��˱���ÿ��һ�뷢��һ�Ρ�
	*
	* \return ���������غ�ͨ�� decode_type ���жϽ����������ͣ��ڷֱ��union�л�ȡ��Ӧ����Ϣ�����.
	*			���decode_type�Ľ��Ϊdecode_error_type , ��ʾ���Ľ������
	*
	* \see	adsb_decode_result_tag
	*
	*/
	ADSB_API adsb_decode_result_t	adsb_decode(char* data, size_t data_size);

	/**
	* \brief �����ݿ�
	*
	* \param [in] filename	���ݿ��ļ�����
	*
	* \return �ɹ������ݿ��ļ��������ݿ�����ʧ�ܷ���NULL ��
	*
	*
	*/
	ADSB_API db_handle open_aircrafts_db(const char *filename);

	/**
	* \brief �����ݿ��в�ѯ������Ϣ
	*
	* \param [in] db		���ݿ���
	* \param [in] icao_addr Ҫ��ѯ�ķ�������ICAO��ַ.
	*
	* \return ��ѯ�ɹ������ص�adsb_aircraft_information_t�ṹ�е�query_result==true , ����Ϊfalse .
	*
	* \see	adsb_aircraft_information_tag
	*
	*/
	ADSB_API adsb_aircraft_information_t query_aircrafts_information(db_handle db, adsb_icao_addr_t icao_addr);

	/**
	* \brief �ر����ݿ��ļ����ͷ�ռ�õ���Դ
	*
	* \param [in] db	���ݿ���.
	*
	*/
	ADSB_API void close_aircrafts_db(db_handle db);

	/**
	* \brief �����豸���������
	*
	* \param [in] param				�������
	* \param [in] current_dev_ip	��ǰ�豸��IP��ַ
	* \param  [in] current_dev_port	��ǰ�豸�Ľ��ն˿�
	*
	*
	*/
	ADSB_API void config_device_net_paramter(dev_net_config param, const char* current_dev_ip, unsigned short current_dev_port);


	enum state_e { HEADL, HEADF, BSYNC1, BSYNC2, SYN1, SYN2, SOH1, TXT, CRC1, CRC2, END };

#define ACARS_MAX_TXT_LEN (256)

	typedef struct acars_msg
	{
		unsigned char mode;                     //ACARSЭ��ģʽ A����B����1����ʾģʽA����2����ʾģʽB�� ����0X31��0X32
		unsigned char addr[8];                  //�ɻ�ע���
		unsigned char ack;                      //������ȷ�� ���ַ���A-Z��a-z����NAK(0X15)
		unsigned char label[3];                 //��Ϣ��ǩ ��ͬ�ı�ǩ����Ϣ����������Ϣ�����ǲ�ͬ�ġ�
		unsigned char bid;                      //������·ʶ��� ��Ϣ�������ֿ�����ʱ��ÿ�δ�����Ϣ�ı�š���Χ��0-9
		unsigned char no[5];                    //��Ϣ��
		unsigned char fid[7];                   //�����
		char txt[ACARS_MAX_TXT_LEN];            //��Ϣ����
		std::string recv_time_str;              //����ʱ��

	} acars_msg_t;

	typedef struct acars_mstat
	{
		state_e state;                          //��Ϣ����״̬
		int ind;                                //��Ϣ����ƫ�� ������Ϣ����ʱ������Ϣ״̬��ʼ���߽�����Ϣ����ʱָ����Ϣƫ��
		unsigned short crc;                     //У��
		char txt[ACARS_MAX_TXT_LEN];            //��Ϣ����
	} acars_mstat_s;


	typedef void(*acars_recv_callback)(const acars_msg_t&);
	/**
	* \brief ����ACARS����ɹ��Ļص�����
	*
	* \param [in] cb				�ص�����ָ��
	*
	*/
	ADSB_API void acars_set_decode_callback(acars_recv_callback cb);
	/**
	* \brief ACARS���뺯��
	*
	* \param [in] data_ptr				ACARS����
	* \param [in] data_size				ACARS���ݳ���(��ǰ��Ϊ�̶�ֵ:246)
	*
	*/
	ADSB_API void acars_decode(const char* data_ptr, unsigned int data_size);

#ifdef __cplusplus
}
#endif
#endif