
#include "acars_utility.h"
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
namespace TH_UTILITY
{


void TrimRightNotAlnum(char* str , int len)
{
    for(int i= len -1 ; i>=0 ; i--)
    {
        if(!isalnum(str[i]))
            str[i] = 0;
    }
}

int Bit_Count(unsigned char data)
{
    unsigned int count=0;
    while(data!=0)
    {
        data &=(data-1);
        count++;
    }
    return count;
}

std::string time2String(time_t tmp_time)
{
    struct tm *p;  
    p = localtime(&tmp_time);
    p->tm_year = p->tm_year + 1900;
    p->tm_mon = p->tm_mon + 1;

    char temp[20];
    int len = sprintf(temp,  "%04d-%02d-%02d %02d-%02d-%02d", p->tm_year, p->tm_mon, p->tm_mday,p->tm_hour,p->tm_min,p->tm_sec);

    return std::string(temp, len);
}

}//namespace
