#include "DS1302.h"

// 引脚定义，和硬件严格对应
sbit DS1302_IO   = P3^4;   
sbit DS1302_CE   = P1^7;   
sbit DS1302_SCLK = P3^5; 

// DS1302各时间寄存器的写地址定义
#define DS1302_SECOND  0X80  // 秒寄存器写地址
#define DS1302_MINUTE  0X82  // 分寄存器写地址
#define DS1302_HOUR    0X84  // 时寄存器写地址
#define DS1302_DATE    0X86  // 日寄存器写地址
#define DS1302_MONTH   0X88  // 月寄存器写地址
#define DS1302_DAY     0X8A  // 星期寄存器写地址
#define DS1302_YEAR    0X8C  // 年寄存器写地址
#define DS1302_WP      0X8E  // 写保护寄存器地址

// 时间缓存数组，格式：[年,月,日,时,分,秒,星期]
unsigned char DS1302_Time[] = {25, 6, 30, 10, 30, 0, 2};

/**
 * @brief  DS1302初始化函数
 */
void DS1302_Init(void)
{
	DS1302_CE = 0;    // 拉低片选，禁用DS1302
	DS1302_SCLK = 0;  // 拉低时钟，初始化时
}

/**
 * @brief  向DS1302写入一个字节数据
 * @param  command: 寄存器命令/地址（含写标识）
 * @param  date: 要写入的单字节数据（BCD码格式）
 */
void DS1302_WriteByte(unsigned char command, unsigned char date)
{
	unsigned char i = 0;
	DS1302_CE = 1;  // 拉高片选，使能DS1302通信
	
	// 第一步：写入8位命令/地址（低位先行）
	for(i = 0; i < 8; i++)
	{
		DS1302_IO = command & (0x01 << i);
		DS1302_SCLK = 1;
		DS1302_SCLK = 0;
	}
	
	// 第二步：写入8位数据（低位先行）
	for(i = 0; i < 8; i++)
	{
		DS1302_IO = date & (0x01 << i);
		DS1302_SCLK = 1;
		DS1302_SCLK = 0;
	}
	
	DS1302_CE = 0;  // 拉低片选，结束本次写操作
}

/**
 * @brief  从DS1302读取一个字节数据
 * @param  command: 寄存器命令/地址（写地址，函数内自动转为读地址）
 * @retval 读取到的单字节数据（BCD码格式）
 */
unsigned char DS1302_ReadByte(unsigned char command)
{
	unsigned char i, date = 0x00;
	command |= 0x01;   // 将写地址转为读地址（最低位置1）
	DS1302_CE = 1;     // 拉高片选，使能DS1302通信
	
	// 第一步：发送8位读地址（低位先行）
	for(i = 0; i < 8; i++)
	{
		DS1302_IO = command & (0x01 << i);
		DS1302_SCLK = 0;
		DS1302_SCLK = 1;
	}
	
	// 第二步：接收8位数据（低位先行）
	for(i = 0; i < 8; i++)
	{
		DS1302_SCLK = 1;
		DS1302_SCLK = 0;
		if(DS1302_IO)
		{
			date |= (0x01 << i);
		} 
	}
	
	DS1302_CE = 0;
	DS1302_IO = 0;     // 复位数据引脚，避免干扰
	return date;
}

/**
 * @brief  设置DS1302实时时钟时间，从全局数组提取
 * @note   强制清除秒寄存器CH停机位，保证时钟持续走时
 */
void DS1302_SetTime(void)
{
	DS1302_WriteByte(DS1302_WP, 0x00);  // 关闭写保护
	
	DS1302_WriteByte(DS1302_YEAR,   DS1302_Time[0]/10*16 + DS1302_Time[0]%10);
	DS1302_WriteByte(DS1302_MONTH,  DS1302_Time[1]/10*16 + DS1302_Time[1]%10);
	DS1302_WriteByte(DS1302_DATE,   DS1302_Time[2]/10*16 + DS1302_Time[2]%10);
	DS1302_WriteByte(DS1302_HOUR,   DS1302_Time[3]/10*16 + DS1302_Time[3]%10);
	DS1302_WriteByte(DS1302_MINUTE, DS1302_Time[4]/10*16 + DS1302_Time[4]%10);
	// 秒寄存器强制清CH位（bit7），保证时钟不停
	DS1302_WriteByte(DS1302_SECOND, (DS1302_Time[5]/10*16 + DS1302_Time[5]%10) & 0x7F);
	DS1302_WriteByte(DS1302_DAY,    DS1302_Time[6]/10*16 + DS1302_Time[6]%10);
	
	DS1302_WriteByte(DS1302_WP, 0x80);  // 开启写保护，防止误写
}

/**
 * @brief  读取DS1302实时时钟时间，存入全局数组
 * @note   秒寄存器屏蔽CH停机位，避免数值读取错误
 */
void DS1302_ReadTime(void)
{
	unsigned char sec;
	
	DS1302_Time[0] = DS1302_ReadByte(DS1302_YEAR)/16*10   + DS1302_ReadByte(DS1302_YEAR)%16;
	DS1302_Time[1] = DS1302_ReadByte(DS1302_MONTH)/16*10  + DS1302_ReadByte(DS1302_MONTH)%16;
	DS1302_Time[2] = DS1302_ReadByte(DS1302_DATE)/16*10   + DS1302_ReadByte(DS1302_DATE)%16;
	DS1302_Time[3] = DS1302_ReadByte(DS1302_HOUR)/16*10   + DS1302_ReadByte(DS1302_HOUR)%16;
	DS1302_Time[4] = DS1302_ReadByte(DS1302_MINUTE)/16*10 + DS1302_ReadByte(DS1302_MINUTE)%16;
	
	// 秒寄存器屏蔽最高位CH停机位
	sec = DS1302_ReadByte(DS1302_SECOND) & 0x7F;
	DS1302_Time[5] = sec/16*10 + sec%16;
	
	DS1302_Time[6] = DS1302_ReadByte(DS1302_DAY)/16*10 + DS1302_ReadByte(DS1302_DAY)%16;
}