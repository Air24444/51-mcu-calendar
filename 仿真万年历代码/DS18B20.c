#include "DS18B20.h"
#include <INTRINS.H>

sbit DS18B20_DQ = P3^2;

// 12MHz 精准10us级延时，适配单总线时序
static void Delay10us(unsigned char t)
{
    while(t--)
    {
        _nop_();_nop_();_nop_();_nop_();_nop_();
        _nop_();_nop_();_nop_();
    }
}

// 毫秒级延时
static void Delay_ms(unsigned int ms)
{
    unsigned int i,j;
    for(i=ms; i>0; i--)
        for(j=120; j>0; j--);
}

// 复位初始化，返回1=应答成功，0=失败
bit DS18B20_Init(void)
{
    bit ack;
    DS18B20_DQ = 1;
    _nop_();
    DS18B20_DQ = 0;
    Delay10us(50);  // 500us 标准复位脉冲
    DS18B20_DQ = 1;
    Delay10us(6);   // 60us 后检测应答
    ack = DS18B20_DQ;
    Delay10us(44);  // 补齐剩余时序
    return ~ack;
}

// 写一字节
static void DS18B20_Write(unsigned char dat)
{
    unsigned char i;
    for(i=0; i<8; i++)
    {
        DS18B20_DQ = 0;
        _nop_();_nop_();
        DS18B20_DQ = dat & 0x01;
        Delay10us(6);
        DS18B20_DQ = 1;
        dat >>= 1;
    }
}

// 读一字节
static unsigned char DS18B20_Read(void)
{
    unsigned char i, dat=0;
    for(i=0; i<8; i++)
    {
        DS18B20_DQ = 0;
        _nop_();_nop_();
        dat >>= 1;
        DS18B20_DQ = 1;
        _nop_();_nop_();_nop_();
        if(DS18B20_DQ) dat |= 0x80;
        Delay10us(6);
    }
    return dat;
}

// 读取整数温度
unsigned char DS18B20_GetTemp(void)
{
    unsigned char tl, th;
    int temp;
    
    // 初始化失败返回99，方便调试
    if(DS18B20_Init() == 0) return 99;
    
    DS18B20_Write(0xCC);   // 跳过ROM
    DS18B20_Write(0x44);   // 启动温度转换
    // 1000ms，确保12位精度下转换100%完成
    Delay_ms(1000);
    // =============================================
    
    if(DS18B20_Init() == 0) return 99;
    DS18B20_Write(0xCC);
    DS18B20_Write(0xBE);   // 读暂存器
    
    tl = DS18B20_Read();
    th = DS18B20_Read();
    
    // 拼接16位值，去掉4位小数
    temp = ((int)th << 8) | tl;
    temp = temp >> 4;
    
    // 合理范围过滤
    if(temp < -10 || temp > 125) return 99;
    if(temp < 0) return 0;
    
    return (unsigned char)temp;
}