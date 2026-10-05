#include "Lunar.h"

// ============================================================
// 2000～2099 农历数据（每年3项：闰月月份、年总天数、月大小位表）
// ============================================================
code unsigned char Lunar_LeapMonth[100] = {
    0,4,0,0,2,0,7,0,0,5,   // 2000-2009
    0,0,4,0,9,0,0,6,0,0,   // 2010-2019
    4,0,0,2,0,6,0,0,5,0,   // 2020-2029
    0,4,0,6,0,0,0,7,0,5,   // 2030-2039
    0,6,0,0,0,7,0,5,0,6,   // 2040-2049
    0,0,0,3,0,7,0,5,0,6,   // 2050-2059
    0,0,4,0,6,0,0,7,0,0,   // 2060-2069
    4,0,0,3,0,7,0,0,6,0,   // 2070-2079
    0,4,0,0,2,0,6,0,0,5,   // 2080-2089
    0,0,3,0,6,0,0,5,0,0    // 2090-2099
};

code unsigned int Lunar_YearDays[100] = {
    354,384,354,355,384,354,384,354,355,384,   // 2000-2009
    354,355,384,354,384,354,355,384,354,355,   // 2010-2019
    384,354,355,384,354,384,354,355,384,354,   // 2020-2029
    355,384,354,384,355,354,355,384,354,384,   // 2030-2039
    355,384,354,355,384,354,384,354,355,384,   // 2040-2049
    354,355,384,354,384,354,355,384,354,384,   // 2050-2059
    355,354,384,355,384,354,355,384,354,384,   // 2060-2069
    384,354,355,384,354,384,355,354,384,355,   // 2070-2079
    384,354,384,354,384,355,384,354,355,384,   // 2080-2089
    354,384,384,354,384,355,354,384,355,384    // 2090-2099
};

// 月大小位表（小端序，bit0 对应正月，1=30天，0=29天）
code unsigned int Lunar_MonthBits[100] = {
    0x0B52,0x04AE,0x0A57,0x054D,0x0D26,0x0D95,0x1655,0x056A,0x09AD,0x055D,
    0x04AE,0x0A5B,0x0A4D,0x0D25,0x1D25,0x0B54,0x0D6A,0x0ADA,0x095B,0x1497,
    0x0497,0x0A4B,0x0B4B,0x06A5,0x06D4,0x1AB5,0x02B6,0x0957,0x052F,0x0497,
    0x0656,0x0D4A,0x0EA5,0x0B55,0x056A,0x1A5B,0x025D,0x092D,0x0D2B,0x0A95,
    0x0B55,0x06CA,0x0B55,0x1535,0x04DA,0x0A5D,0x1457,0x052D,0x0A9A,0x0E95,
    0x06AA,0x0AEA,0x0AB5,0x04B6,0x0AAE,0x0A57,0x0526,0x0F26,0x0D95,0x05B5,
    0x056A,0x096D,0x04DD,0x04AD,0x0A4D,0x0D4D,0x0D25,0x0D55,0x0B54,0x0B5A,
    0x195A,0x095B,0x049B,0x0A97,0x0A4B,0x0B27,0x06A5,0x06D4,0x0AF4,0x0AB6,
    0x0957,0x04AF,0x064B,0x074A,0x0EA5,0x06B5,0x055C,0x0AB6,0x096D,0x092E,
    0x0C95,0x0D4A,0x1D8A,0x0B55,0x056A,0x1A5B,0x025D,0x092D,0x0D2B,0x0A95
};

// -----------------------------------------------------------
// 获取农历某月天数（月份 1~12，is_leap 指示是否闰月）
// -----------------------------------------------------------
static unsigned char GetLunarMonthDays(unsigned char year_idx, unsigned char month, bit is_leap)
{
    unsigned int bits = Lunar_MonthBits[year_idx];
    unsigned char leap = Lunar_LeapMonth[year_idx];
    if (is_leap && leap == month)
        return ((bits >> (month - 1)) & 1) ? 30 : 29;
    else
        return ((bits >> (month - 1)) & 1) ? 30 : 29;
}

// -----------------------------------------------------------
// 公历转农历主函数
// -----------------------------------------------------------
void SolarToLunar(unsigned char solar_y, unsigned char solar_m, unsigned char solar_d, Lunar_Type *lunar)
{
    unsigned long total = 0;
    unsigned char i, y_idx, leap_m;
    unsigned int full_year = 2000 + solar_y;
    bit is_leap = 0;
    unsigned char month_days[] = {31,28,31,30,31,30,31,31,30,31,30,31};

    // 限制日期下限（2000年2月5日前一律视为该日）
    if (solar_y == 0 && (solar_m < 2 || (solar_m == 2 && solar_d < 5)))
    {
        solar_y = 0; solar_m = 2; solar_d = 5;
    }

    // 计算从2000年1月1日到目标日期的天数
    for (i = 0; i < solar_y; i++)
    {
        if (((2000 + i) % 4 == 0 && (2000 + i) % 100 != 0) || (2000 + i) % 400 == 0)
            total += 366;
        else
            total += 365;
    }
    for (i = 0; i < solar_m - 1; i++)
        total += month_days[i];
    if (((full_year % 4 == 0 && full_year % 100 != 0) || full_year % 400 == 0) && solar_m > 2)
        total += 1;
    total += solar_d;

    // 减去2000年2月5日（农历2000年正月初一）的偏移量（35天）
    if (total >= 35)
        total -= 35;
    else
        total = 0;

    // 确定农历年份
    y_idx = 0;
    while (1)
    {
        unsigned int year_days = Lunar_YearDays[y_idx];
        if (total < year_days) break;
        total -= year_days;
        y_idx++;
        if (y_idx >= 100) { y_idx = 99; break; }
    }
    lunar->year = 2000 + y_idx;

    // 确定农历月、日
    leap_m = Lunar_LeapMonth[y_idx];
    lunar->month = 1;
    while (1)
    {
        unsigned char days = GetLunarMonthDays(y_idx, lunar->month, 0);
        if (total < days) break;
        total -= days;

        if (lunar->month == leap_m && !is_leap)
        {
            days = GetLunarMonthDays(y_idx, lunar->month, 1);
            if (total < days)
            {
                is_leap = 1;
                break;
            }
            total -= days;
        }
        lunar->month++;
        if (lunar->month > 12) break;
    }

    lunar->day = (unsigned char)(total + 1);
    lunar->leap_month = is_leap ? lunar->month : 0;
}