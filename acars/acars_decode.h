/*******************************************************************************
** Copyright (c):
** 创建人:                   邵保国
** 日 期:                  2015-01-04
** 修改人:
** 日 期:
** 描 述:   acars报文解码
** 说 明:   
*******************************************************************************/

#ifndef _ACARS_DECODE_H_
#define _ACARS_DECODE_H_

#include "../adsb_decoder_api.h"
#include "singleton.h"
#include <fstream>

class acars_decoder
{
    //声明为单例对象
    DECLARE_SINGLETON_CLASS(acars_decoder);

	//数据处理回调函数
	//typedef boost::function<void(const acars_msg_t&)> acars_recv_callback;


public:
    acars_decoder();

    int getmesg(unsigned char r);

    void init_mesg();
    void set_callback(acars_recv_callback acars_rc);
    void set_recv_time(const std::string & time_str);

    ~acars_decoder();
private:
    void update_crc(unsigned short *crc, unsigned char ch);
    int build_mesg(char *txt, int len);

    void correct_by_onebyte(char *txt, int len);
private:
    //接收报文的当前状态
    acars_mstat_s _curr_msg_state;

private:
    unsigned int _crc_error_num;//crc校验出错数据个数
    unsigned int _all_msg_num;  //总数据个数
    unsigned int _crc_correct_num;//crc能够纠正正确的数据个数
    char _source_txt[ACARS_MAX_TXT_LEN]; //去掉校验位之前的acars消息
    unsigned char _source_crc1;          //原始crc1
    unsigned char _source_crc2;          //原始crc2

    std::ofstream _statistics_file;

    std::string _recv_time;//HH:MM:SS 接收时间
    acars_recv_callback _acars_recv;
};

typedef pattern::singleton<acars_decoder> sln_acars_decoder ; 

#endif
