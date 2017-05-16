/*******************************************************************************
** Copyright (c):
** 创建人:                   邵保国
** 日 期:                  2014-11-4
** 修改人:
** 日 期:
** 描 述:   相关辅助函数
** 说 明:   有些函数可能不是跨平台函数，如需跨平台，需重新实现
*******************************************************************************/

#ifndef _CUTERADIO_UTILITY_H_
#define _CUTERADIO_UTILITY_H_
#include <string>

namespace TH_UTILITY
{


//删除末尾的非字母，非数字字符
void TrimRightNotAlnum(char* str , int len);

//查找一个char中二进制1的个数
int Bit_Count(unsigned char data);

//时间戳转字符串
std::string time2String(time_t tmp_time);

#define BIT_SET(a,b) ((a) |= (1<<(b)))          //置位
#define BIT_CLEAR(a,b) ((a) &= ~(1<<(b)))       //清除位
#define BIT_FLIP(a,b) ((a) ^= (1<<(b)))         //位翻转
#define BIT_CHECK(a,b) ((a) & (1<<(b)))         //校验
}
#endif
