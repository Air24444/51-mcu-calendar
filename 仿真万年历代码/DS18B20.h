#ifndef __DS18B20_H__
#define __DS18B20_H__

#include <REGX52.H>

bit DS18B20_Init(void);       // 初始化，返回1=成功 0=失败
unsigned char DS18B20_GetTemp(void); // 返回整数温度值

#endif