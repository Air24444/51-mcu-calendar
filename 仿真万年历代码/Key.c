#include "Key.h"

// 按键引脚定义，对应原理图P3口四个独立按键
sbit KEY_SET_PIN     = P2^3;   // 设置键
sbit KEY_SEL_PIN     = P2^2;   // 选择键
sbit KEY_ADD_PIN     = P2^1;   // 加键
sbit KEY_SUB_LUNAR_PIN = P2^0; // 减/农历切换键

// 按键消抖专用延时
static void Key_DelayMs(unsigned int ms)
{
    unsigned int i, j;
    for(i = ms; i > 0; i--)
        for(j = 110; j > 0; j--);
}

unsigned char Key_Scan(void)
{
    unsigned char key = KEY_NONE;
    P2_3=0;
	P2_2=0;
	P2_1=0;
	P2_0=0;
    // 逐个检测按键，低电平有效，松手触发
    if(KEY_SET_PIN == 1)
    {
        Key_DelayMs(20);          // 消抖
        while(KEY_SET_PIN == 0);  // 等待按键松开
        Key_DelayMs(20);
        key = KEY_SET;
    }
    else if(KEY_SEL_PIN == 1)
    {
        Key_DelayMs(20);
        while(KEY_SEL_PIN == 1);
        Key_DelayMs(20);
        key = KEY_SEL;
    }
    else if(KEY_ADD_PIN == 1)
    {
        Key_DelayMs(20);
        while(KEY_ADD_PIN == 1);
        Key_DelayMs(20);
        key = KEY_ADD;
    }
    else if(KEY_SUB_LUNAR_PIN == 1)
    {
        Key_DelayMs(20);
        while(KEY_SUB_LUNAR_PIN == 1);
        Key_DelayMs(20);
        key = KEY_SUB_LUNAR;
    }
    
    return key;
}