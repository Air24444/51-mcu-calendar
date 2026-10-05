#ifndef __DEVICE_CTRL_H__
#define __DEVICE_CTRL_H__

#include <REGX52.H>

void DeviceCtrl_Init(void);
void AlarmFan_Control(unsigned char temp, unsigned char threshold);
void DeviceCtrl_StopAll(void);
#endif
