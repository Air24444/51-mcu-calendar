#include "DeviceCtrl.h"

sbit BEEP = P1^6;   // PNP驱动，低电平响
sbit FAN  = P1^2;   // NPN驱动，高电平转

void DeviceCtrl_Init(void) {
    BEEP = 1;   // 初始关闭蜂鸣器
    FAN  = 1;   // 初始关闭风扇
}

void AlarmFan_Control(unsigned char temp, unsigned char threshold) {
    // 温度传感器故障返回 99，或值非法时关闭设备
    if(temp == 99 || temp > 125) {
        BEEP = 1;
        FAN  = 0;
        return;
    }
    if(temp >= threshold) {
        BEEP = 0;
        FAN  = 0;
    } else {
        BEEP = 1;
        FAN  = 1;
    }
}
// 新增：强制关闭所有报警外设
void DeviceCtrl_StopAll(void) {
    BEEP = 1;   // 关闭蜂鸣器
    FAN  = 1;   // 关闭风扇
}