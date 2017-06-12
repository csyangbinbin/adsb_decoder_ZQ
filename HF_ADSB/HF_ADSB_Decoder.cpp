#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#ifdef WIN32
#include <winsock2.h>
#include <Ws2ipdef.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif
#include "tinyxml/tinystr.h"  
#include "tinyxml/tinyxml.h"  
#include "adsb_decoder_api.h"

#ifdef WIN32
#pragma comment(lib,"ws2_32.lib")
#endif


using namespace std;

const char* MULTICAST_IP = "224.1.1.11"; //多播组地址
const int MULTICAST_PORT = 5302; //多播组端口
const int BUFFER_SIZE = 1024;


void process_recv_data(char* data, size_t data_size);
void on_error_type(const adsb_decode_result_t* ret);
void on_air_location_type(const adsb_decode_result_t* ret);
void on_surface_location_type(const adsb_decode_result_t* ret);
void on_identification_type(const adsb_decode_result_t* ret);
void on_velocity_type(const adsb_decode_result_t* ret);
void on_dev_board_info_type(const adsb_decode_result_t* ret);
void dump_common_info(const adsb_common_info_t*	comm_info);


#ifndef WIN32
typedef int SOCKET ; 
#endif
SOCKET SendSocket;
std::string recvAddr = "127.0.0.1";
sockaddr_in RecvAddr;
int recvPort = 27015;

static void sendXML2Net(const std::string& xmlstr)
{
	if(SendSocket)
	{
		sendto(SendSocket,
			xmlstr.c_str(),
			xmlstr.size(),
			0,
			(struct sockaddr*)&RecvAddr,
			sizeof(RecvAddr));
	}
}

int main(int argc ,char** argv)
{
if(argc!=3)
{
	printf("usage:%s IP_Address Port\r\n" , argv[0]);
	return 0; 
}

#ifdef WIN32
WSADATA wsaData;
WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif


recvAddr = argv[1] ;
recvPort = atoi(argv[2]);
printf("Send XML Data To %s:%d\r\n" , recvAddr.c_str() , recvPort);


	int ret;
	SOCKET server;
	SendSocket = 0;
	SendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);


	RecvAddr.sin_family = AF_INET;
	RecvAddr.sin_port = htons(recvPort);
	RecvAddr.sin_addr.s_addr = inet_addr(recvAddr.c_str() );

	server = socket(AF_INET, SOCK_DGRAM, 0); //创建一个UDP套接口
	printf("create socket: %d\r\n", server);


	sockaddr_in local;
	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_port = htons(MULTICAST_PORT);
	local.sin_addr.s_addr = INADDR_ANY;

	ret = bind(server, (sockaddr*)(&local), sizeof(local));
	if (ret <0)
	{
		return 0;
	}

	ip_mreq mreq;
	memset(&mreq, 0, sizeof(mreq));
	mreq.imr_interface.s_addr = INADDR_ANY;
	mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_IP);

	ret = setsockopt(server, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq));
	if (ret<0)
	{
		return 0;
	}

	char buf[BUFFER_SIZE + 1];
	sockaddr_in client;

#ifdef WIN32
	typedef int socklen_t;
#endif
	socklen_t clientLen;

	for (;;)
	{
		clientLen = sizeof(client);
		memset(&client, 0, clientLen);

		ret = recvfrom(server, buf, BUFFER_SIZE, 0, (sockaddr*)(&client), &clientLen);
		if (ret == 0)
		{
			continue;
		}
		else if (ret <0)
		{
			break;
		}

		process_recv_data(buf, ret);
	}


}

void process_recv_data(char* data, size_t data_size)
{
	if (data == NULL || data_size == 0)
		return;

	adsb_decode_result_t	ret = adsb_decode(data, data_size);  //调用解码函数进行解码

	switch (ret.decode_type)
	{
	case decode_error_type:
//		on_error_type(&ret); //错误的数据报文
		break;
	case air_location_type:
//		on_air_location_type(&ret); //空中位置信息报文
		break;
	case surface_location_type:
//		on_surface_location_type(&ret); //地面位置信息报文
		break;
	case identification_type:
		on_identification_type(&ret); //航班号信息报文
		break;
	case velocity_type:
//		on_velocity_type(&ret); //速度信息报文
		break;
	case dev_board_info_type:
//		on_dev_board_info_type(&ret);//接收机状态信息报文
		break;
	}

	printf("-----------------------------------------\r\n");
}


void addTextNode(TiXmlElement *pParentNode ,std::string nodeName ,std::string nodeText)
{
	TiXmlElement *Node = new TiXmlElement(nodeName);
	pParentNode->LinkEndChild(Node);
	TiXmlText *pStrText = new TiXmlText(nodeText);
	Node->LinkEndChild(pStrText);
}


void dump_common_info(const adsb_common_info_t*	comm_info , TiXmlElement *rootElement = NULL )
{
	char buffer[255];
	sprintf(buffer , "%02X%02X%02X" ,
		comm_info->icao_addr.m1,
		comm_info->icao_addr.m2,
		comm_info->icao_addr.m3);

	addTextNode(rootElement , "icao" , buffer);

	
	sprintf(buffer,"%s", comm_info->timestamp.valid ? "Y" : "N");
	addTextNode(rootElement, "timeValid", buffer);


	sprintf(buffer , "%04d/%02d/%02d %02d:%02d:%02d" , 
		comm_info->timestamp.tm_year,
		comm_info->timestamp.tm_mon,
		comm_info->timestamp.tm_mday,
		comm_info->timestamp.tm_hour,
		comm_info->timestamp.tm_min,
		comm_info->timestamp.tm_sec);
	addTextNode(rootElement, "timestamp", buffer);

	printf("ICAO:%02X%02X%02X\r\n",
		comm_info->icao_addr.m1,
		comm_info->icao_addr.m2,
		comm_info->icao_addr.m3);

	printf("%s-%04d/%02d/%02d %02d:%02d:%02d %03d:%03d:%03d\r\n",
		comm_info->timestamp.valid ? "Y" : "N",
		comm_info->timestamp.tm_year,
		comm_info->timestamp.tm_mon,
		comm_info->timestamp.tm_mday,
		comm_info->timestamp.tm_hour,
		comm_info->timestamp.tm_min,
		comm_info->timestamp.tm_sec,
		comm_info->timestamp.tv_msec,
		comm_info->timestamp.tv_wsec,
		comm_info->timestamp.tv_nsec);

	//基站编号
	printf("StationID:%04X\r\n", comm_info->station_id);

}


std::string xml2str(TiXmlElement *node)
{
	TiXmlPrinter printer;
	string xmlstr;
	node->Accept(&printer);
	xmlstr = printer.CStr();
	return  xmlstr;
}

void on_error_type(const adsb_decode_result_t* ret)
{
	printf("Type: error_type\r\n");
}

void on_air_location_type(const adsb_decode_result_t* ret)
{

	TiXmlElement *messageRoot = new TiXmlElement("message");
	if (messageRoot == NULL)
		return; 
	messageRoot->SetAttribute("type", "airpos");

	printf("Type: air_location_type\r\n");
	dump_common_info(&ret->decode_result.air_location_info.comm_info , messageRoot);



	printf("longitude:%.6f\r\n", ret->decode_result.air_location_info.longitude); //目标的经度
	printf("latitude:%.6f\r\n", ret->decode_result.air_location_info.latitude); //目标的纬度
	printf("altitude:%.2f m\r\n", ret->decode_result.air_location_info.altitude); //目标的高度


	char buffer[100];
	sprintf(buffer,"%.6f" , ret->decode_result.air_location_info.longitude);
	addTextNode(messageRoot, "longitude", buffer);
	sprintf(buffer, "%.6f", ret->decode_result.air_location_info.latitude);
	addTextNode(messageRoot, "latitude", buffer);
	sprintf(buffer, "%.6f", ret->decode_result.air_location_info.altitude);
	addTextNode(messageRoot, "altitude", buffer);

	std::string xmlStr = xml2str(messageRoot);

	if (messageRoot)
	{
		delete messageRoot;
		messageRoot = NULL; 
	}
	
	sendXML2Net(xmlStr);
}

void on_surface_location_type(const adsb_decode_result_t* ret)
{

	TiXmlElement *messageRoot = new TiXmlElement("message");
	if (messageRoot == NULL)
		return;
	messageRoot->SetAttribute("type", "surfacePos");

	printf("Type: surface_location_type\r\n");
	dump_common_info(&ret->decode_result.surface_location_info.comm_info , messageRoot);


	char buffer[100];
	sprintf(buffer, "%.6f", ret->decode_result.surface_location_info.longitude);
	addTextNode(messageRoot, "longitude", buffer);

	sprintf(buffer, "%.6f", ret->decode_result.surface_location_info.latitude);
	addTextNode(messageRoot, "latitude", buffer);

	sprintf(buffer, "%.2f", ret->decode_result.surface_location_info.movement);
	addTextNode(messageRoot, "movement", buffer);

	sprintf(buffer, "%s", ret->decode_result.surface_location_info.heading_state?"Y":"N");
	addTextNode(messageRoot, "headingState", buffer);

	sprintf(buffer, "%.1f", ret->decode_result.surface_location_info.heading_value);
	addTextNode(messageRoot, "headingValue", buffer);
	std::string xmlStr = xml2str(messageRoot);
	if (messageRoot)
	{
		delete messageRoot;
		messageRoot = NULL;
	}

	sendXML2Net(xmlStr);

	printf("longitude:%.6f\r\n", ret->decode_result.surface_location_info.longitude); //地面目标的经度
	printf("latitude:%.6f\r\n", ret->decode_result.surface_location_info.latitude); //地面目标的纬度
	printf("movement:%.2f Km/h\r\n", ret->decode_result.surface_location_info.movement);//地面目标的行驶速度
	printf("heading_state:%d\r\n", ret->decode_result.surface_location_info.heading_state); //地面目标的行驶方向信息是否可用
	printf("heading_value:%.1f\r\n", ret->decode_result.surface_location_info.heading_value); //地面目标的行驶方向值
}

void on_identification_type(const adsb_decode_result_t* ret)
{
	TiXmlElement *messageRoot = new TiXmlElement("message");
	if (messageRoot == NULL)
		return;
	messageRoot->SetAttribute("type", "IDCategory");


	printf("Type: identification_type\r\n");
	dump_common_info(&ret->decode_result.id_info.comm_info , messageRoot);

	char flightid[9];
	memset(flightid, 0, 9);
	memcpy(flightid, ret->decode_result.id_info.flight_id, 8);
	printf("ID:%s\r\n", flightid);				//航班号(ICAO三字代码)
	printf("category_set:%d\r\n", ret->decode_result.id_info.category_set); //类别集
	printf("category:%d\r\n", ret->decode_result.id_info.category); //类别

	char buffer[255];
	sprintf(buffer, "%s", flightid);
	addTextNode(messageRoot, "flightID", buffer);

	sprintf(buffer, "%d", ret->decode_result.id_info.category_set);
	addTextNode(messageRoot, "categorySet", buffer);

	sprintf(buffer, "%d", ret->decode_result.id_info.category);
	addTextNode(messageRoot, "category", buffer);
	std::string xmlStr = xml2str(messageRoot);
	if (messageRoot)
	{
		delete messageRoot;
		messageRoot = NULL;
	}

	sendXML2Net(xmlStr);
}

void on_velocity_type(const adsb_decode_result_t* ret)
{
	TiXmlElement *messageRoot = new TiXmlElement("message");
	if (messageRoot == NULL)
		return;
	messageRoot->SetAttribute("type", "velocity");


	printf("Type: velocity_type\r\n");
	dump_common_info(&ret->decode_result.velocity_info.comm_info , messageRoot);

	printf("gs_value:%.2f kts\r\n", ret->decode_result.velocity_info.gs_value); //地速(单位:节 kts)
	printf("gs_heading:%.2f\r\n", ret->decode_result.velocity_info.gs_heading); //地速方向(单位:角度) (负值标志方向信息无效)
	printf("vrate:%d  ft/min\r\n", ret->decode_result.velocity_info.vrate); //升速(单位: 英尺/分钟 ft/min) ,正值为上升，负值为下降

	char buffer[255];
	sprintf(buffer, "%.2f", ret->decode_result.velocity_info.gs_value);
	addTextNode(messageRoot, "value", buffer);

	sprintf(buffer, "%.2f", ret->decode_result.velocity_info.gs_heading);
	addTextNode(messageRoot, "heading", buffer);

	sprintf(buffer, "%d", ret->decode_result.velocity_info.vrate);
	addTextNode(messageRoot, "vrate", buffer);
	std::string xmlStr = xml2str(messageRoot);
	if (messageRoot)
	{
		delete messageRoot;
		messageRoot = NULL;
	}
	sendXML2Net(xmlStr);
}

void on_dev_board_info_type(const adsb_decode_result_t* ret)
{

	TiXmlElement *messageRoot = new TiXmlElement("message");
	if (messageRoot == NULL)
		return;
	messageRoot->SetAttribute("type", "boardInfo");


	printf("Type: dev_board_info_type\r\n");

	bool test_data_valid = (ret->decode_result.board_info.valid_flag&FLAG_TEST_DATA_MASK) != 0;	//是否为测试数据报文
	bool station_id_valid = (ret->decode_result.board_info.valid_flag&FLAG_STATION_ID_MASK) != 0;	//接收机编号是否有效
	bool gps_ymd_valid = (ret->decode_result.board_info.valid_flag&FLAG_GPS_YMD_MASK) != 0;		//GPS信息中的年月日信息是否有效
	bool gps_hms_valid = (ret->decode_result.board_info.valid_flag&FLAG_GPS_HMS_MASK) != 0;		//GPS信息中的时分秒信息是否有效
	bool satellite_count_valid = (ret->decode_result.board_info.valid_flag&FLAG_GPS_SATELLITE_COUNT_MASK) != 0;//GPS搜星数量信息是否有效
	bool gps_location_valid = (ret->decode_result.board_info.valid_flag&FLAG_GPS_LOCATION_MASK) != 0;		//GPS定位信息是否有效
	bool battery_info_valid = (ret->decode_result.board_info.valid_flag&FLAG_BATTERY_INFO_MASK) != 0;		//电池的状态信息是否有效
	bool board_temp_valid = (ret->decode_result.board_info.valid_flag&FLAG_BOARD_TEMP_MASK) != 0;		//电路板的温度信息是否有效
	bool channel_state_valid = (ret->decode_result.board_info.valid_flag&FLAG_CHANNEL_STATE_MASK) != 0;	//通道状态信息是否有效
	bool pcb_version_valid = (ret->decode_result.board_info.valid_flag&FLAG_PCB_VERSION_MASK) != 0;		//PCB版本信息是否有效
	bool firmware_version_valid = (ret->decode_result.board_info.valid_flag&FLAG_FIRMWARE_VERSION_MASK) != 0; //固件版本信息是否有效
	bool gps_velocity_valid = (ret->decode_result.board_info.valid_flag&FLAG_GPS_VELOCITY_MASK) != 0;		//GPS速度信息是否有效

	printf("FLAG_TEST_DATA_MASK: %s\r\n", test_data_valid ? "Y" : "N");
	printf("FLAG_STATION_ID_MASK: %s\r\n", station_id_valid ? "Y" : "N");
	printf("FLAG_GPS_YMD_MASK: %s\r\n", gps_ymd_valid ? "Y" : "N");
	printf("FLAG_GPS_HMS_MASK: %s\r\n", gps_hms_valid ? "Y" : "N");
	printf("FLAG_GPS_SATELLITE_COUNT_MASK: %s\r\n", satellite_count_valid ? "Y" : "N");
	printf("FLAG_GPS_LOCATION_MASK: %s\r\n", gps_location_valid ? "Y" : "N");
	printf("FLAG_BATTERY_INFO_MASK: %s\r\n", battery_info_valid ? "Y" : "N");
	printf("FLAG_BOARD_TEMP_MASK: %s\r\n", board_temp_valid ? "Y" : "N");
	printf("FLAG_CHANNEL_STATE_MASK: %s\r\n", channel_state_valid ? "Y" : "N");
	printf("FLAG_PCB_VERSION_MASK: %s\r\n", pcb_version_valid ? "Y" : "N");
	printf("FLAG_FIRMWARE_VERSION_MASK: %s\r\n", firmware_version_valid ? "Y" : "N");
	printf("FLAG_GPS_VELOCITY_MASK: %s\r\n", gps_velocity_valid ? "Y" : "N");



	char buffer[255];
	sprintf(buffer, "%s", test_data_valid ? "Y" : "N");
	addTextNode(messageRoot, "FLAG_TEST_DATA_MASK", buffer);

	sprintf(buffer, "%s", station_id_valid ? "Y" : "N");
	addTextNode(messageRoot, "FLAG_STATION_ID_MASK", buffer);

	sprintf(buffer, "%s", gps_ymd_valid ? "Y" : "N");
	addTextNode(messageRoot, "FLAG_GPS_YMD_MASK", buffer);

	sprintf(buffer, "%s", gps_hms_valid ? "Y" : "N");
	addTextNode(messageRoot, "FLAG_GPS_HMS_MASK", buffer);

	sprintf(buffer, "%s", satellite_count_valid ? "Y" : "N");
	addTextNode(messageRoot, "FLAG_GPS_SATELLITE_COUNT_MASK", buffer);

	sprintf(buffer, "%s", gps_location_valid ? "Y" : "N");
	addTextNode(messageRoot, "FLAG_GPS_LOCATION_MASK", buffer);

	sprintf(buffer, "%s", battery_info_valid ? "Y" : "N");
	addTextNode(messageRoot, "FLAG_BATTERY_INFO_MASK", buffer);

	sprintf(buffer, "%s", board_temp_valid ? "Y" : "N");
	addTextNode(messageRoot, "FLAG_BOARD_TEMP_MASK", buffer);

	sprintf(buffer, "%s", channel_state_valid ? "Y" : "N");
	addTextNode(messageRoot, "FLAG_CHANNEL_STATE_MASK", buffer);

	sprintf(buffer, "%s", pcb_version_valid ? "Y" : "N");
	addTextNode(messageRoot, "FLAG_PCB_VERSION_MASK", buffer);

	sprintf(buffer, "%s", firmware_version_valid ? "Y" : "N");
	addTextNode(messageRoot, "FLAG_FIRMWARE_VERSION_MASK", buffer);

	sprintf(buffer, "%s", gps_velocity_valid ? "Y" : "N");
	addTextNode(messageRoot, "FLAG_GPS_VELOCITY_MASK", buffer);


	//接收机编号信息可用
	//if (station_id_valid)
	//{
		printf("station id: %04X\r\n", ret->decode_result.board_info.station_id);

		sprintf(buffer, "%04X", ret->decode_result.board_info.station_id);
		addTextNode(messageRoot, "stationID", buffer);
	//}

	//GPS中年月日时间可用
	if (gps_ymd_valid)
	{
		dev_board_gps_info_t gps_info = ret->decode_result.board_info.gps_info;
		printf("GPS YMD:%04d/%02d/%02d\r\n", gps_info.year, gps_info.month, gps_info.day);


		sprintf(buffer, "%04d/%02d/%02d", gps_info.year, gps_info.month, gps_info.day);
		addTextNode(messageRoot, "gpsDate", buffer);
	}

	//GPS中时分秒信息可用
	if (gps_hms_valid)
	{
		dev_board_gps_info_t gps_info = ret->decode_result.board_info.gps_info;
		printf("GPS HMS:%02d:%02d:%02d\r\n", gps_info.hour, gps_info.minute, gps_info.second);

		sprintf(buffer, "%02d:%02d:%02d", gps_info.hour, gps_info.minute, gps_info.second);
		addTextNode(messageRoot, "gpsTime", buffer);
	}

	//GPS搜星数量信息可用
	if (satellite_count_valid)
	{
		dev_board_gps_info_t gps_info = ret->decode_result.board_info.gps_info;
		printf("satellite count: %d\r\n", gps_info.satellite_count);

		sprintf(buffer, "%d", gps_info.satellite_count);
		addTextNode(messageRoot, "satelliteCount", buffer);
	}

	//GPS定位信息可用
	if (gps_location_valid)
	{
		dev_board_gps_info_t gps_info = ret->decode_result.board_info.gps_info;
		printf("GPS location: %.6f	%.6f	%.2f\r\n", gps_info.longitude, gps_info.latitude, gps_info.altitude);



		sprintf(buffer, "%.6f,%.6f,%.2f", gps_info.longitude, gps_info.latitude, gps_info.altitude);
		addTextNode(messageRoot, "gpsPos", buffer);
	}

	//GPS测得的速度信息是否可用
	if (gps_velocity_valid)
	{
		dev_board_gps_info_t gps_info = ret->decode_result.board_info.gps_info;
		printf("GPS velocity: %.2f	%.2f\r\n", gps_info.velocity_heading, gps_info.velocity_value);

		sprintf(buffer, "%.2f,%.2f", gps_info.velocity_heading, gps_info.velocity_value);
		addTextNode(messageRoot, "gpsVel", buffer);
	}

	//电池信息是否可用
	if (battery_info_valid)
	{
		dev_board_battery_info_t battery_info = ret->decode_result.board_info.battery_info;

		printf("battery state: %d\r\n", battery_info.state); //充电或者放电(1--放电 , 0--充电)
		printf("battery capability: %d%% \r\n", battery_info.capability); //当前电量的百分比(取值:0 ~ 100)
		printf("battery voltage:%.1f\r\n", battery_info.voltage); //电池当前电压
		printf("battery current:%.1f\r\n", battery_info.current); //电池当前电流
		printf("battry work time: %d\r\n", battery_info.work_time); //充放电时间
		printf("battery temp:%d\r\n", battery_info.battery_temperature); //电池温度

		sprintf(buffer, "%d,%d,%.1f,%.1f,%d,%d", 
			battery_info.state , battery_info.capability ,
			battery_info.voltage , battery_info.current ,
			battery_info.work_time , battery_info.battery_temperature);
		addTextNode(messageRoot, "batteryInfo", buffer);

	}

	//硬件板卡温度信息可用
	if (board_temp_valid)
	{
		printf("board temp: %.1f\r\n", ret->decode_result.board_info.board_temperature);

		sprintf(buffer, "%.1f",ret->decode_result.board_info.board_temperature);
		addTextNode(messageRoot, "boardTemp", buffer);
	}

	//信号通道状态信息可用
	if (channel_state_valid)
	{
		dev_channel_state_info_t channel_info = ret->decode_result.board_info.channel_state;
		printf("channel id: %d\r\n", channel_info.channel_id);
		printf("channel state: %d\r\n", channel_info.channel_state);

		sprintf(buffer, "%d,%d", channel_info.channel_id , channel_info.channel_state);
		addTextNode(messageRoot, "channelState", buffer);
	}

	//PCB版本号信息可用
	if (pcb_version_valid)
	{
		printf("pcb version: %d\r\n", ret->decode_result.board_info.dev_pcb_version);
		sprintf(buffer, "%d", ret->decode_result.board_info.dev_pcb_version);
		addTextNode(messageRoot, "pcbVersion", buffer);
	}

	//固件版本信息可用
	if (firmware_version_valid)
	{
		printf("firmware version_valid: %d\r\n", ret->decode_result.board_info.dev_firmware_version);
		sprintf(buffer, "%d", ret->decode_result.board_info.dev_firmware_version);
		addTextNode(messageRoot, "firmwareVersion", buffer);
	}


	std::string xmlStr = xml2str(messageRoot);
	if (messageRoot)
	{
		delete messageRoot;
		messageRoot = NULL;
	}

	sendXML2Net(xmlStr);
}

