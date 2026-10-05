##ifndef __DS1302_H__
#define __DS1302_H__

#include <REGX52.H>

// 时间缓存数组，格式索引：0=年 1=月 2=日 3=时 4=分 5=秒 6=星期
extern unsigned char DS1302_Time[];

void DS1302_Init(void);
void DS1302_WriteByte(unsigned char command, unsigned char date);
unsigned char DS1302_ReadByte(unsigned char command);
void DS1302_SetTime(void);
void DS1302_ReadTime(void);

#endif