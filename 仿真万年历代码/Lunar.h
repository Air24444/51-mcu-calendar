#ifndef __LUNAR_H__
#define __LUNAR_H__

typedef struct {
    unsigned int  year;        // 农历年份（如 2033）
    unsigned char month;       // 农历月（1-12）
    unsigned char day;         // 农历日
    unsigned char leap_month;  // 闰月月份（0 表示无闰月）
} Lunar_Type;

// 公历转农历，输入公历年（0~99 对应 2000~2099）、月、日
void SolarToLunar(unsigned char solar_y, unsigned char solar_m, unsigned char solar_d, Lunar_Type *lunar);

#endif