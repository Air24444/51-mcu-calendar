#include <REGX52.H>
// 定义I2C总线的时钟线和数据线
sbit I2C_SCL = P3^7;
sbit I2C_SDA = P3^6;

/**
 * @brief  发送I2C通信起始信号
 * @note   遵循I2C标准时序：SCL高电平时，SDA产生下降沿（高→低），启动一次I2C传输
 * @param  无
 * @retval 无
 */
void I2C_Start()
{
	I2C_SCL=1;    // 先将时钟线置高，为起始信号做准备
	I2C_SDA=1;    // 先将数据线置高，保证总线处于空闲状态
	I2C_SDA=0;    // 关键：SCL高电平时，SDA由高变低，产生起始信号
	I2C_SCL=0;    // 时钟线置低，结束起始信号，准备后续数据传输
}

/**
 * @brief  发送I2C通信停止信号
 * @note   遵循I2C标准时序：SCL高电平时，SDA产生上升沿（低→高），终止一次I2C传输并释放总线
 * @param  无
 * @retval 无
 */
void I2C_Stop()
{
	I2C_SDA=0;    // 先将数据线置低，为停止信号做准备
	I2C_SCL=1;    // 时钟线置高，为SDA电平变化提供时序保障
	I2C_SDA=1;    // 关键：SCL高电平时，SDA由低变高，产生停止信号，释放I2C总线
}

/**
 * @brief  向I2C外设发送1字节（8位）数据
 * @note   遵循I2C高位先行的传输规则，逐位发送数据，通过SCL时钟锁存
 * @param  Byte：要发送的1字节无符号字符数据
 * @retval 无
 */
void I2C_SendByte(unsigned char Byte)
{
	unsigned char i=0;
	// 循环8次，依次发送字节的每一位（从最高位到最低位）
	for(i=0;i<8;i++)
	{
		I2C_SDA=Byte&(0x80>>i);  // 提取当前要发送的位（0x80是最高位掩码，右移i位定位到对应位）
		I2C_SCL=1;                // 拉高时钟线，让外设读取SDA总线上的当前数据位
		I2C_SCL=0;                // 拉低时钟线，准备发送下一位数据
	}
}

/**
 * @brief  从I2C外设接收1字节（8位）数据
 * @note   遵循I2C高位先行的传输规则，逐位读取数据，读取前需释放SDA总线
 * @param  无
 * @retval 接收成功的1字节无符号字符数据
 */
unsigned char I2C_ReceiveByte()
{
	unsigned char Byte=0x00;  // 定义接收缓存变量，初始化为0
	unsigned char i=0;
	I2C_SDA=1;                // 读取前先释放SDA总线（置高），由外设控制SDA电平变化
	
	// 循环8次，依次读取字节的每一位（从最高位到最低位）
	for(i=0;i<8;i++)
	{
		I2C_SCL=1;                // 拉高时钟线，锁定SDA总线上的当前数据位，准备读取
		if(I2C_SDA==1)            // 判断当前SDA电平，读取对应数据位
		{
			Byte|=(0x80>>i);       // 若SDA为高，将对应位赋值到接收缓存（按位或赋值拼接数据）
		}
		// 若SDA为低，无需操作，Byte对应位保持0
		I2C_SCL=0;                // 拉低时钟线，准备读取下一位数据
	}
	return Byte;                 // 返回接收完成的1字节数据
}

/**
 * @brief  向I2C外设发送应答（ACK）或非应答（NACK）信号
 * @note   I2C协议规定：接收方接收1字节后，需返回应答信号告知发送方
 *         ACK=0：有效应答（表示接收成功，请求继续传输）
 *         ACK=1：非应答（表示接收完成，终止传输）
 * @param  ACK：应答标志位（0=应答，1=非应答）
 * @retval 无
 */
void I2C_SendACK(unsigned char ACK)
{
	I2C_SDA=ACK;   // 将应答信号赋值到SDA总线
	I2C_SCL=1;     // 拉高时钟线，让外设读取应答信号
	I2C_SCL=0;     // 拉低时钟线，完成应答信号发送
}

/**
 * @brief  从I2C外设接收应答（ACK）或非应答（NACK）信号
 * @note   I2C协议规定：发送方发送1字节后，需读取接收方的应答信号判断传输是否成功
 *         返回0：外设应答（传输成功）
 *         返回1：外设非应答（传输失败或无需继续传输）
 * @param  无
 * @retval 应答状态（0=应答，1=非应答）
 */
unsigned char I2C_ReceiveACK()
{
	unsigned char ACK=0;  // 定义应答状态缓存变量
	I2C_SDA=1;            // 读取前先释放SDA总线（置高），由外设控制应答电平
	I2C_SCL=1;            // 拉高时钟线，锁定外设返回的应答电平，准备读取
	ACK=I2C_SDA;          // 读取SDA电平，保存应答状态
	I2C_SCL=0;            // 拉低时钟线，完成应答信号接收
	return ACK;           // 返回应答状态
}