#include <REGX52.H>
#include "deLay.h"   
#include "i2c.h"     // I2C总线驱动头文件

// AT24C512的I2C设备地址定义（硬件A2/A1/A0接地时，与AT24C02地址一致）
#define AT24C512write  0XA0  // 写操作设备地址：1010 0000（二进制）
#define AT24C512read   0XA1  // 读操作设备地址：1010 0001（二进制）
sbit I2C_SCL = P3^7;
sbit I2C_SDA = P3^6;
/**
 * @brief  AT24C512初始化函数
 * @note   初始化I2C总线引脚，确保总线处于空闲初始状态
 * @param  无
 * @retval 无
 */
void AT24C512_Init(void)
{
    I2C_SCL = 1;
    I2C_SDA = 1;
}

/**
 * @brief  向AT24C512指定内部地址写入1字节数据
 * @note   严格遵循AT24C512字节写时序：起始→设备写地址→应答→地址高8位→应答→地址低8位→应答→数据→应答→停止
 * @param  WriteAdress：AT24C512内部存储地址（范围0x0000~0xFFFF，共64KB）
 * @param  Date：要写入的1字节数据（0x00~0xFF）
 * @retval 无
 */
void AT24C512_WriteDate(unsigned int WriteAdress, unsigned char Date)
{
	I2C_Start();                      // 发送I2C起始信号（SCL高时SDA降沿），启动通信
	I2C_SendByte(AT24C512write);      // 发送AT24C512写设备地址，通知芯片准备接收数据
	while(I2C_ReceiveACK() == 1);     // 等待AT24C512应答：无应答则循环等待，直到收到应答
	I2C_SendByte(WriteAdress >> 8);   // 发送16位存储地址的高8位（AT24C512容量更大，需双字节地址寻址）
	while(I2C_ReceiveACK() == 1);     // 等待地址高字节应答
	I2C_SendByte(WriteAdress & 0xFF); // 发送16位存储地址的低8位
	while(I2C_ReceiveACK() == 1);     // 等待地址低字节应答
	I2C_SendByte(Date);               // 发送要写入的1字节数据
	while(I2C_ReceiveACK() == 1);     // 等待数据应答，确保数据发送成功
	I2C_Stop();                       // 发送I2C停止信号，结束写通信
	Delay(5);                         // 延时5ms，等待EEPROM内部数据固化（写操作必须等待内部写入周期）
}

/**
 * @brief  从AT24C512指定内部地址读取1字节数据
 * @note   严格遵循AT24C512随机读时序：起始→设备写地址→应答→双字节地址→应答→二次起始→设备读地址→应答→读数据→非应答→停止
 * @param  ReadAdress：AT24C512内部存储地址（范围0x0000~0xFFFF）
 * @retval 读取到的1字节数据（0x00~0xFF）；通信失败时返回0xFF（需结合硬件排查）
 */
unsigned char AT24C512_ReceiveDate(unsigned int ReadAdress)
{
	unsigned char Date = 0X00;        // 定义数据接收缓存，初始化为0
	
	I2C_Start();                      // 第一步起始：先通过写操作指定要读取的内部地址
	I2C_SendByte(AT24C512write);      // 发送写设备地址
	while(I2C_ReceiveACK() == 1);     // 等待设备应答
	I2C_SendByte(ReadAdress >> 8);    // 发送16位存储地址的高8位
	while(I2C_ReceiveACK() == 1);     // 等待应答
	I2C_SendByte(ReadAdress & 0xFF);  // 发送16位存储地址的低8位
	while(I2C_ReceiveACK() == 1);     // 等待应答
	
	I2C_Start();                      // 第二步二次起始：切换为读操作模式
	I2C_SendByte(AT24C512read);       // 发送读设备地址，通知芯片准备输出数据
	while(I2C_ReceiveACK() == 1);     // 等待设备应答
	Date = I2C_ReceiveByte();         // 接收芯片返回的1字节数据
	I2C_SendACK(1);                   // 发送非应答（1=非应答），通知芯片停止发送
	I2C_Stop();                       // 发送停止信号，结束读通信
	
	return Date;                      // 返回读取到的数据
}