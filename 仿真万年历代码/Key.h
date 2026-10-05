#ifndef __KEY_H__
#define __KEY_H__

#include <REGX52.H>

// 键值返回值定义
#define KEY_NONE        0
#define KEY_SET         1
#define KEY_SEL         2
#define KEY_ADD         3
#define KEY_SUB_LUNAR   4

unsigned char Key_Scan(void);

#endif
