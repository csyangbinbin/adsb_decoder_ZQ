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


	/** 接收机硬件发送的UDP组播数据包的长度 */
#define ADSB_MESSAGE_PACKAGE_LEN			62


	/**
	* \brief 接收ADS-B数据报文的时间戳信息
	*/
	typedef struct adsb_timestamp_tag
	{
		u8	valid;		///<时间戳信息是否有效，如果未接GPS/BD硬件，则时间无效
		u32 tm_sec;		///< 秒.	[0-60]  ,注意以下时间均为UTC时间
		u32 tm_min;		///< 分钟.	[0-59] 
		u32 tm_hour;	///< 小时.	[0-23] 
		u32 tm_mday;	///< 天.	[1-31] 
		u32 tm_mon;		///< 月.	[1-12] 
		u32 tm_year;	///< 年.  
		u32 tv_msec;	///< 毫秒
		u32 tv_wsec;	///< 微秒
		u32 tv_nsec;	///< 纳秒
	}adsb_timestamp_t;


	/**
	* \brief ADS-B目标ICAO编号表示
	*
	*	ICAO地址由三个字节组成,例如780A1F ,m1=0x78 , m2=0x0A ,m3=0x1F
	*/
	typedef struct adsb_icao_addr_tag
	{
		u8 m1;	///< 高位字节
		u8 m2;	///< 中间字节
		u8 m3;	///< 低位字节
	} adsb_icao_addr_t;

	/**
	* \brief ADS-B消息通用结构
	*
	*/
	typedef struct adsb_common_info_tag
	{
		adsb_icao_addr_tag  icao_addr;		///< ICAO地址
		adsb_timestamp_t	timestamp;		///< 时间戳
		unsigned short		station_id;		///< 接收机编号
		unsigned int		squawk;			///< 应答机编号
	} adsb_common_info_t;

	/**
	* \brief ADS-B空中位置信息
	*
	*/
	typedef struct adsb_air_location_info_tag
	{
		adsb_common_info_t	comm_info;		///< 通用信息
		double	longitude;					///< 经度(单位:角度)
		double	latitude;					///< 纬度(单位:角度)
		double	altitude;					///< GPS高度(单位:米)
	}adsb_air_location_info_t;

	/**
	* \brief ADS-B地面位置信息
	*
	*/
	typedef struct adsb_surface_location_info_tag
	{
		adsb_common_info_t	comm_info;			///< 通用信息
		double	longitude;						///< 经度(单位:角度)
		double	latitude;						///< 纬度(单位:角度)
		double	movement;						///< 地面运行速度(单位：千米/小时 km/h)
		u8		heading_state;					///< 机头朝向信息是否可用(0-不可用 , 1-可用)
		double	heading_value;					///< 机头朝向(单位:角度)
	}adsb_surface_location_info_t;


	/**
	* \brief 飞行器的身份识别信息
	*
	*/
	typedef struct	adsb_identification_info_tag
	{
		adsb_common_info_t	comm_info;				///< 通用信息
		s8 flight_id[8];							///< 航班号

		/**
		*	飞行器类别信息
		*
		*	1 = Aircraft identification, Category Set D	\n
		*	2 = Aircraft identification, Category Set C \n
		*	3 = Aircraft identification, Category Set B \n
		*	4 = Aircraft identification, Category Set A	\n
		*
		*/
		u8 category_set;

		/**
		* ADS-B发射器类别编码 \n
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
		1 = Surface Vehicle – Emergency Vehicle	\n
		2 = Surface Vehicle – Service Vehicle	\n
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
	* \brief ADS-B速度信息
	*
	*/
	typedef struct	adsb_velocity_info_tag
	{
		adsb_common_info_t	comm_info;				///< 通用信息
		double	gs_value;							///< 地速(单位:节 kts)
		double	gs_heading;							///< 地速方向(单位:角度) (负值标志方向信息无效)
		s32		vrate;								///< 升速(单位: 英尺/分钟 ft/min) ,正值为上升，负值为下降
	} adsb_velocity_info_t;


	/**
	*	\brief 设备电池信息
	*
	* 有些型号的ADS-B接收设备中安装有蓄电池，以便户外没有市电的情况下使用。
	* 如果您使用的设备中不包含电池，请忽略此电池信息
	*
	*/
	typedef struct dev_board_battery_info_tag
	{
		u8					state;				///< 充电或者放电(1--放电 , 0--充电)
		u8					capability;		///< 当前电量的百分比(取值:0 ~ 100)
		double				voltage;			///< 电池当前电压
		double				current;			///< 电池当前电流
		u16					work_time;			/**< 表示电池剩余时间，若电池处于放电状态 ,表示电池剩余的工作时间，
												若电池处于充电状态表示电池充满电还需多长时间	*/
		s8					battery_temperature;///< 电池温度(正数表示零上，负数表示零下)
	}dev_board_battery_info_t;

	/**
	*	\brief 设备上连接的GPS/北斗定位系统信息
	*
	* 有些型号的ADS-B接收设备中安装有GPS/北斗定位模块，具有定位和授时的功能。
	* 如果您使用的设备中不包GPS/北斗设备，请忽略此信息
	*
	*/
	typedef struct dev_board_gps_info_tag
	{
		u8			satellite_count;		///< GPS/BD搜星数量

		double		longitude;			///< 经度(单位:角度) 
		double		latitude;			///< 纬度(单位:角度)
		double		altitude;			///< 高度(单位:米)

		double		velocity_heading;		///< GPS/BD速度方向,以正北为参考基准(单位:角度)
		double		velocity_value;			///< GPS/BD速度大小(单位:节 knots)

		u32			year;					///< 年 (UTC 时间)
		u32			month;					///< 月 (UTC 时间)
		u32			day;					///< 日 (UTC 时间)
		u32			hour;					///< 时 (UTC 时间)
		u32			minute;					///< 分 (UTC 时间)
		u32			second;				///< 秒 (UTC 时间)
	}dev_board_gps_info_t;

	/** ADS-B通道工作正常 */
#define ADSB_CHANNEL_STATE_WORKING		0

	/** ADS-B通道未使用 */
#define ADSB_CHANNEL_STATE_UNUSED		1

	/** ADS-B通道故障 */
#define ADSB_CHANNEL_STATE_FAULT		2

	/** ADS-B通道状态未知 */
#define ADSB_CHANNEL_STATE_UNDEFINED	3

	/**
	*	\brief 设备各路信号通道状态
	*
	*	本ADS-B接收设备包含多路信号接收通道，但所有通道不一定都处于正常工作状态。
	*
	*/
	typedef struct dev_channel_state_info_tag
	{
		u8	channel_id;		///< 通道ID
		u8	channel_state;		///< 通道状态
	} dev_channel_state_info_t;


	/**
	*	\brief ADS-B接收设备硬件状态信息
	*
	*	本系统每秒钟会发送一此设备的状态信息，其中包含有设备的温度，GPS信息，电池信息，
	*	通道状态，软硬件版本号等信息,如果长时间没有接收到此信息则表明设备出现故障，或者网络
	*	连接出现问题。
	*/
	typedef struct dev_board_info_tag
	{
		/**
		* 两字节的信息有效标志位，表示哪些信息是有效的。
		*
		* 两个字节的标志位含义，从高位到低位依次为: \n
		* bit15 - 是否为测试数据 \n
		* bit14 - 接收机编号信息是否有效 \n
		* bit13 - GPS信息中年月日信息是否有效 \n
		* bit12 - GPS信息中时分秒信息是否有效 \n
		* bit11 - GPS搜星数量信息是否有效 \n
		* bit10 - GPS经纬高信息是否有效 \n
		* bit9 -  内部使用数据，保留 \n
		* bit8 -  内部使用数据，保留 \n
		* bit7 -  内部使用数据，保留 \n
		* bit6 -  电池信息是否有效 \n
		* bit5 -  设备温度信息是否有效 \n
		* bit4 -  信号处理通道状态信息是否有效 \n
		* bit3 -  设备硬件电路版本号信息是否有效 \n
		* bit2 -  设备硬件固件版本号信息是否有效 \n
		* bit1 -  GPS速度信息是否有效 \n
		* bit0 -  内部使用数据，保留 \n
		*/
		u16							valid_flag;
		u16							station_id;			///< 接收机编号(每个接收机唯一)
		dev_board_gps_info_t		gps_info;				///< GPS/BD定位信息
		dev_board_battery_info_t	battery_info;			///< 电池信息	
		double						board_temperature;		///< 设备温度
		dev_channel_state_info_t	channel_state;			///< 信号处理通道状态
		u8							dev_pcb_version;		///< 设备硬件电路版本号
		u8							dev_firmware_version;	///< 设备硬件固件版本号
	}dev_board_info_t;


	/** 是否为测试数据掩码 */
#define FLAG_TEST_DATA_MASK						(1<<15)

	/** 接收机编号信息是否有效掩码 */
#define FLAG_STATION_ID_MASK					(1<<14)

	/** GPS信息中年月日信息是否有效掩码 */
#define FLAG_GPS_YMD_MASK						(1<<13)

	/** 信息中时分秒信息是否有效掩码 */
#define FLAG_GPS_HMS_MASK						(1<<11)

	/** GPS搜星数量信息是否有效掩码 */
#define FLAG_GPS_SATELLITE_COUNT_MASK			(1<<11)

	/** GPS经纬高信息是否有效掩码 */
#define FLAG_GPS_LOCATION_MASK					(1<<10)

	/** 电池信息是否有效掩码 */
#define FLAG_BATTERY_INFO_MASK					(1<<6)

	/** 设备温度信息是否有效掩码 */
#define FLAG_BOARD_TEMP_MASK					(1<<5)

	/** 信号处理通道状态信息是否有效掩码 */
#define FLAG_CHANNEL_STATE_MASK					(1<<4)

	/** 设备硬件电路版本号信息是否有效掩码 */
#define FLAG_PCB_VERSION_MASK					(1<<3)

	/** 设备硬件固件版本号信息是否有效掩码 */
#define FLAG_FIRMWARE_VERSION_MASK				(1<<2)

	/** GPS速度信息是否有效掩码 */
#define FLAG_GPS_VELOCITY_MASK					(1<<1)

	/**
	*	报文解码结果类型
	*/
	enum adsb_decode_type
	{
		decode_error_type = 0,	///< 错误的无法解码的报文
		air_location_type,		///< ADS-B空中位置信息报文
		surface_location_type,	///< ADS-B地面位置信息报文
		identification_type,	///< ADS-B航班号信息报文
		velocity_type,			///< ADS-B速度信息报文
		dev_board_info_type		///< 周期性设备状态信息报文
	};

	/**
	*	\brief 报文解码结果
	*
	*/
	typedef struct adsb_decode_result_tag
	{
		adsb_decode_type					decode_type;					///< 解码结果类型
		union
		{
			adsb_air_location_info_t		air_location_info;				///< ADS-B空中位置信息报文
			adsb_surface_location_info_t	surface_location_info;			///< ADS-B地面位置信息报文
			adsb_identification_info_t		id_info;						///< ADS-B航班号信息报文
			adsb_velocity_info_t			velocity_info;					///< ADS-B速度信息报文
			dev_board_info_t				board_info;					///< 周期性设备状态信息报文
		} decode_result;		///< 解码结果
	}adsb_decode_result_t;

#define air_location_info_				decode_result.air_location_info
#define id_info_						decode_result.id_info
#define surface_location_info_			decode_result.surface_location_info
#define velocity_info_					decode_result.velocity_info
#define board_info_						decode_result.board_info


	/**
	*	\brief 从数据库中查询的目标飞行器的静态信息结果
	*
	*/
	typedef struct adsb_aircraft_information_tag
	{
		adsb_icao_addr_t icao;	///< 飞行器的ICAO地址
		bool	query_result;	///< 数据库中是否包含指定ICAO的目标的信息
		char registration[20];	///< 飞行器的注册号
		char manufacturer[40]; ///< 飞行器的制造商
		char model[5];			///< 飞行器型号简称,如B738
		char model_details[40];	///< 飞行器型号的详细信息
		char country_iso[3];	///< 飞行器所属国家的ISO代码,如中国CN,美国US
	}adsb_aircraft_information_t;


	enum dev_config_enable_flag
	{
		dev_ip_addr_enable_flag = 0x80, //设备自身的IP地址配置更改
		dev_config_port_enable_flag = 0x40, //设备自身监听端口配置更改
		dev_mac_addr_enable_flag = 0x20, //设备自身的MAC地址配置更改
		dest_ip_addr_enable_flag = 0x10, //接收目的端的IP地址配置更改
		dest_recv_port = 0x08,	//接收目的端的接收端口配置更改
		dev_config_all_enable = 0xF8 //所有配置更改
	};

	/**
	*	\brief 设备网络配置参数
	*
	*/
	typedef struct dev_net_config_tag
	{
		char				dev_ip_addr[16]; //设备自身的IP地址
		unsigned short		dev_config_port; //设备自身监听端口(用于配置操作或PING操作)
		unsigned char		dev_mac_addr[6]; //设备自身的MAC地址（6个字节）
		char				dest_ip_addr[16]; //接收目的端的IP地址
		unsigned short		dest_recv_port;  //接收目的端的接收端口
		unsigned char		config_enable_flag;//配置参数有效标志位
	}dev_net_config;


	typedef void* db_handle;

	/**
	* \brief 对网络接收的ADS-B数据报文进行解码
	*
	* \param [in] data	指向接收的报文的内存指针.
	* \param [in] data_size 接收的报文的长度.
	*
	* \note
	* 当前硬件接收机发送的报文采用UDP组播协议，每个报文的长度固定为62个字节。
	* 报文分为两种类型：\n 1.ADS-B数据报文 \n 2.硬件接收机状态信息报文，此报文每隔一秒发送一次。
	*
	* \return 解码结果返回后，通过 decode_type 域判断解码结果的类型，在分别从union中获取对应的信息结果体.
	*			如果decode_type的结果为decode_error_type , 表示报文解码错误。
	*
	* \see	adsb_decode_result_tag
	*
	*/
	ADSB_API adsb_decode_result_t	adsb_decode(char* data, size_t data_size);

	/**
	* \brief 打开数据库
	*
	* \param [in] filename	数据库文件名称
	*
	* \return 成功打开数据库文件返回数据库句柄，失败返回NULL 。
	*
	*
	*/
	ADSB_API db_handle open_aircrafts_db(const char *filename);

	/**
	* \brief 从数据库中查询机型信息
	*
	* \param [in] db		数据库句柄
	* \param [in] icao_addr 要查询的飞行器的ICAO地址.
	*
	* \return 查询成功，返回的adsb_aircraft_information_t结构中的query_result==true , 否则为false .
	*
	* \see	adsb_aircraft_information_tag
	*
	*/
	ADSB_API adsb_aircraft_information_t query_aircrafts_information(db_handle db, adsb_icao_addr_t icao_addr);

	/**
	* \brief 关闭数据库文件，释放占用的资源
	*
	* \param [in] db	数据库句柄.
	*
	*/
	ADSB_API void close_aircrafts_db(db_handle db);

	/**
	* \brief 配置设备的网络参数
	*
	* \param [in] param				网络参数
	* \param [in] current_dev_ip	当前设备的IP地址
	* \param  [in] current_dev_port	当前设备的接收端口
	*
	*
	*/
	ADSB_API void config_device_net_paramter(dev_net_config param, const char* current_dev_ip, unsigned short current_dev_port);


	enum state_e { HEADL, HEADF, BSYNC1, BSYNC2, SYN1, SYN2, SOH1, TXT, CRC1, CRC2, END };

#define ACARS_MAX_TXT_LEN (256)

	typedef struct acars_msg
	{
		unsigned char mode;                     //ACARS协议模式 A或者B：“1”表示模式A，“2”表示模式B。 即：0X31或0X32
		unsigned char addr[8];                  //飞机注册号
		unsigned char ack;                      //正向技术确认 该字符是A-Z、a-z或者NAK(0X15)
		unsigned char label[3];                 //消息标签 不同的标签的消息所包含的消息种类是不同的。
		unsigned char bid;                      //下行链路识别号 消息过长，分开传输时，每次传输消息的编号。范围：0-9
		unsigned char no[5];                    //消息号
		unsigned char fid[7];                   //航班号
		char txt[ACARS_MAX_TXT_LEN];            //消息内容
		std::string recv_time_str;              //接收时间

	} acars_msg_t;

	typedef struct acars_mstat
	{
		state_e state;                          //消息接收状态
		int ind;                                //消息接收偏移 用以消息有误时返回消息状态开始或者接收消息内容时指定消息偏移
		unsigned short crc;                     //校验
		char txt[ACARS_MAX_TXT_LEN];            //消息内容
	} acars_mstat_s;


	typedef void(*acars_recv_callback)(const acars_msg_t&);
	/**
	* \brief 设置ACARS解码成功的回调函数
	*
	* \param [in] cb				回调函数指针
	*
	*/
	ADSB_API void acars_set_decode_callback(acars_recv_callback cb);
	/**
	* \brief ACARS解码函数
	*
	* \param [in] data_ptr				ACARS数据
	* \param [in] data_size				ACARS数据长度(当前设为固定值:246)
	*
	*/
	ADSB_API void acars_decode(const char* data_ptr, unsigned int data_size);

#ifdef __cplusplus
}
#endif
#endif