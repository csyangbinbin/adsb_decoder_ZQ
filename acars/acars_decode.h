/*******************************************************************************
** Copyright (c):
** ������:                   �۱���
** �� ��:                  2015-01-04
** �޸���:
** �� ��:
** �� ��:   acars���Ľ���
** ˵ ��:   
*******************************************************************************/

#ifndef _ACARS_DECODE_H_
#define _ACARS_DECODE_H_

#include "../adsb_decoder_api.h"
#include "singleton.h"
#include <fstream>

class acars_decoder
{
    //����Ϊ��������
    DECLARE_SINGLETON_CLASS(acars_decoder);

	//���ݴ���ص�����
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
    //���ձ��ĵĵ�ǰ״̬
    acars_mstat_s _curr_msg_state;

private:
    unsigned int _crc_error_num;//crcУ��������ݸ���
    unsigned int _all_msg_num;  //�����ݸ���
    unsigned int _crc_correct_num;//crc�ܹ�������ȷ�����ݸ���
    char _source_txt[ACARS_MAX_TXT_LEN]; //ȥ��У��λ֮ǰ��acars��Ϣ
    unsigned char _source_crc1;          //ԭʼcrc1
    unsigned char _source_crc2;          //ԭʼcrc2

    std::ofstream _statistics_file;

    std::string _recv_time;//HH:MM:SS ����ʱ��
    acars_recv_callback _acars_recv;
};

typedef pattern::singleton<acars_decoder> sln_acars_decoder ; 

#endif
