/*******************************************************************************
** Copyright (c):
** ������:                   �۱���
** �� ��:                  2014-11-4
** �޸���:
** �� ��:
** �� ��:   ��ظ�������
** ˵ ��:   ��Щ�������ܲ��ǿ�ƽ̨�����������ƽ̨��������ʵ��
*******************************************************************************/

#ifndef _CUTERADIO_UTILITY_H_
#define _CUTERADIO_UTILITY_H_
#include <string>

namespace TH_UTILITY
{


//ɾ��ĩβ�ķ���ĸ���������ַ�
void TrimRightNotAlnum(char* str , int len);

//����һ��char�ж�����1�ĸ���
int Bit_Count(unsigned char data);

//ʱ���ת�ַ���
std::string time2String(time_t tmp_time);

#define BIT_SET(a,b) ((a) |= (1<<(b)))          //��λ
#define BIT_CLEAR(a,b) ((a) &= ~(1<<(b)))       //���λ
#define BIT_FLIP(a,b) ((a) ^= (1<<(b)))         //λ��ת
#define BIT_CHECK(a,b) ((a) & (1<<(b)))         //У��
}
#endif
