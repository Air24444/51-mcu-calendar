#include <REGX52.H>
#include "LCD1602.h"
#include "DS1302.h"
#include "DS18B20.h"
#include "AT24C512.h"
#include "Key.h"
#include "Lunar.h"
#include "DeviceCtrl.h"

// 全局变量
Lunar_Type LunarTime;
unsigned char Temp_Value;
unsigned char Temp_Threshold;

unsigned char Set_Mode = 0;
unsigned char Set_Item = 0;
unsigned char Lunar_Flag = 0;
bit Lunar_NeedClear = 1;    // 切换农历/公历时需要清屏一次

/************************ 工具函数 ************************/
static void Main_DelayMs(unsigned int ms) {
    unsigned int i,j;
    for(i=ms; i>0; i--)
        for(j=110; j>0; j--);
}

static unsigned char CalcWeek(unsigned char y, unsigned char m, unsigned char d) {
    unsigned int full_year = 2000 + y;
    unsigned char week;
    if(m < 3) {
        m += 12;
        full_year--;
    }
    week = (d + 2*m + 3*(m+1)/5 + full_year + full_year/4 - full_year/100 + full_year/400 + 1) % 7;
    if(week == 0) week = 7;
    return week;
}

static unsigned char GetMonthDays(unsigned char y, unsigned char m) {
    if(m == 2) {
        if(((2000+y)%4==0 && (2000+y)%100!=0) || (2000+y)%400==0)
            return 29;
        else
            return 28;
    } else if(m==4 || m==6 || m==9 || m==11) {
        return 30;
    } else {
        return 31;
    }
}

/************************ 显示刷新函数 ************************/
void Display_Refresh(void) {
    if(Set_Mode == 0) {
        if(Lunar_Flag == 0) {   // 公历模式
            if(Lunar_NeedClear) {
                LCD_ShowString(1, 1, "                ");
                Lunar_NeedClear = 0;
            }
            LCD_ShowString(1, 1, "20");
            LCD_ShowNum(1, 3, DS1302_Time[0], 2);
            LCD_ShowChar(1, 5, '-');
            LCD_ShowNum(1, 6, DS1302_Time[1], 2);
            LCD_ShowChar(1, 8, '-');
            LCD_ShowNum(1, 9, DS1302_Time[2], 2);
            LCD_ShowString(1, 12, "W");
            LCD_ShowNum(1, 13, DS1302_Time[6], 1);
            LCD_ShowString(1, 14, "   ");

            LCD_ShowNum(2, 1, DS1302_Time[3], 2);
            LCD_ShowChar(2, 3, ':');
            LCD_ShowNum(2, 4, DS1302_Time[4], 2);
            LCD_ShowChar(2, 6, ':');
            LCD_ShowNum(2, 7, DS1302_Time[5], 2);
            LCD_ShowString(2, 10, "T:");
            LCD_ShowNum(2, 12, Temp_Value, 2);
            LCD_ShowChar(2, 14, 'C');
            LCD_ShowString(2, 15, "  ");
        } else {               // 农历模式
            if(Lunar_NeedClear) {
                LCD_ShowString(1, 1, "                ");
                Lunar_NeedClear = 0;
            }
            LCD_ShowString(1, 1, "NL");
            {
                unsigned char lunar_y = LunarTime.year - 2000;
                if(lunar_y > 99) lunar_y = 99;
                LCD_ShowNum(1, 4, lunar_y, 2);
            }
            if(LunarTime.leap_month)
                LCD_ShowChar(1, 7, 'R');
            else
                LCD_ShowChar(1, 7, ' ');
            LCD_ShowNum(1, 8, LunarTime.month, 2);
            LCD_ShowChar(1, 10, 'M');
            LCD_ShowNum(1, 11, LunarTime.day, 2);
            LCD_ShowChar(1, 13, 'D');
            LCD_ShowString(1, 14, "   ");

            // 第二行
            LCD_ShowString(2, 1, "Temp:");
            LCD_ShowNum(2, 6, Temp_Value, 2);
            LCD_ShowChar(2, 8, 'C');
            LCD_ShowString(2, 10, "Lim:");
            LCD_ShowNum(2, 14, Temp_Threshold, 2);
            LCD_ShowChar(2, 16, 'C');
        }
    } else {   // 设置模式
        LCD_ShowString(1, 1, "Set:");
        switch(Set_Item) {
            case 0: LCD_ShowString(1, 5, "Year            "); break;
            case 1: LCD_ShowString(1, 5, "Month           "); break;
            case 2: LCD_ShowString(1, 5, "Day             "); break;
            case 3: LCD_ShowString(1, 5, "Hour            "); break;
            case 4: LCD_ShowString(1, 5, "Minute          "); break;
            case 5: LCD_ShowString(1, 5, "Temp            "); break;
        }

        LCD_ShowString(2, 1, "Val:");
        switch(Set_Item) {
            case 0:
                LCD_ShowString(2, 5, "20");
                LCD_ShowNum(2, 7, DS1302_Time[0], 2);
                LCD_ShowString(2, 9, "       ");
                break;
            case 1:
                LCD_ShowNum(2, 5, DS1302_Time[1], 2);
                LCD_ShowString(2, 7, "         ");
                break;
            case 2:
                LCD_ShowNum(2, 5, DS1302_Time[2], 2);
                LCD_ShowString(2, 7, "         ");
                break;
            case 3:
                LCD_ShowNum(2, 5, DS1302_Time[3], 2);
                LCD_ShowString(2, 7, "         ");
                break;
            case 4:
                LCD_ShowNum(2, 5, DS1302_Time[4], 2);
                LCD_ShowString(2, 7, "         ");
                break;
            case 5:
                LCD_ShowNum(2, 5, Temp_Threshold, 2);
                LCD_ShowChar(2, 7, 'C');
                LCD_ShowString(2, 8, "        ");
                break;
        }
    }
}

// 按键逻辑处理 
void Key_Process(void) {
    unsigned char key = Key_Scan();
    if(key == KEY_NONE) return;

    switch(key) {
        case KEY_SET:
            if(Set_Mode == 0) {
                Set_Mode = 1;
                Set_Item = 0;
            } else {
                Set_Mode = 0;
                DS1302_Time[6] = CalcWeek(DS1302_Time[0], DS1302_Time[1], DS1302_Time[2]);
                DS1302_SetTime();

                AT24C512_WriteDate(0x0010, DS1302_Time[0]);
                AT24C512_WriteDate(0x0011, DS1302_Time[1]);
                AT24C512_WriteDate(0x0012, DS1302_Time[2]);
                AT24C512_WriteDate(0x0013, DS1302_Time[6]);
                AT24C512_WriteDate(0x0014, DS1302_Time[3]);
                AT24C512_WriteDate(0x0015, DS1302_Time[4]);
                AT24C512_WriteDate(0x0016, DS1302_Time[5]);

                AT24C512_WriteDate(0x0000, Temp_Threshold);
                Main_DelayMs(10);
            }
            break;

        case KEY_SEL:
            if(Set_Mode == 1) {
                Set_Item++;
                if(Set_Item > 5) Set_Item = 0;
            }
            break;

        case KEY_ADD:
            if(Set_Mode == 1) {
                switch(Set_Item) {
                    case 0: DS1302_Time[0]++;  if(DS1302_Time[0]>99) DS1302_Time[0]=0; break;
                    case 1: DS1302_Time[1]++; if(DS1302_Time[1]>12) DS1302_Time[1]=1; break;
                    case 2:
                        if(DS1302_Time[2] >= GetMonthDays(DS1302_Time[0], DS1302_Time[1]))
                            DS1302_Time[2] = 1;
                        else
                            DS1302_Time[2]++;
                        break;
                    case 3: DS1302_Time[3]++;  if(DS1302_Time[3]>23) DS1302_Time[3]=0; break;
                    case 4: DS1302_Time[4]++;if(DS1302_Time[4]>59) DS1302_Time[4]=0; break;
                    case 5: Temp_Threshold++;if(Temp_Threshold>50) Temp_Threshold=20; break;
                }
            }
            break;

        case KEY_SUB_LUNAR:
            if(Set_Mode == 0) {
                Lunar_Flag = !Lunar_Flag;
                Lunar_NeedClear = 1;
            } else {
                switch(Set_Item) {
                    case 0: if(DS1302_Time[0]==0) DS1302_Time[0]=99; else DS1302_Time[0]--; break;
                    case 1: if(DS1302_Time[1]==1) DS1302_Time[1]=12; else DS1302_Time[1]--; break;
                    case 2:
                        if(DS1302_Time[2] == 1)
                            DS1302_Time[2] = GetMonthDays(DS1302_Time[0], DS1302_Time[1]);
                        else
                            DS1302_Time[2]--;
                        break;
                    case 3: if(DS1302_Time[3]==0) DS1302_Time[3]=23; else DS1302_Time[3]--; break;
                    case 4: if(DS1302_Time[4]==0) DS1302_Time[4]=59; else DS1302_Time[4]--; break;
                    case 5: if(Temp_Threshold==20) Temp_Threshold=50; else Temp_Threshold--; break;
                }
            }
            break;
    }
}

/************************ 主函数 ************************/
void main(void) {
    unsigned int refresh_cnt = 0;
    unsigned int temp_cnt = 0;
    unsigned int lunar_cnt = 0;

    DeviceCtrl_Init();
    LCD_Init();
    DS1302_Init();
    DS18B20_Init();
    AT24C512_Init();

    // 读取温度阈值，无效则设默认值
    Temp_Threshold = AT24C512_ReceiveDate(0x0000);
    if(Temp_Threshold < 20 || Temp_Threshold > 50) {
        Temp_Threshold = 30;
        AT24C512_WriteDate(0x0000, Temp_Threshold);
        Main_DelayMs(10);
    }

    // 从 EEPROM 恢复上次时间
    DS1302_Time[0] = AT24C512_ReceiveDate(0x0010);
    DS1302_Time[1] = AT24C512_ReceiveDate(0x0011);
    DS1302_Time[2] = AT24C512_ReceiveDate(0x0012);
    DS1302_Time[6] = AT24C512_ReceiveDate(0x0013);
    DS1302_Time[3] = AT24C512_ReceiveDate(0x0014);
    DS1302_Time[4] = AT24C512_ReceiveDate(0x0015);
    DS1302_Time[5] = AT24C512_ReceiveDate(0x0016);

    // 首次上电数据无效则写入默认时间
    if(DS1302_Time[0] > 99) {
        DS1302_Time[0] = 25;
        DS1302_Time[1] = 6;
        DS1302_Time[2] = 30;
        DS1302_Time[6] = CalcWeek(25, 6, 30);
        DS1302_Time[3] = 10;
        DS1302_Time[4] = 30;
        DS1302_Time[5] = 0;
        DS1302_SetTime();
    } else {
        DS1302_SetTime();
    }

    DeviceCtrl_StopAll();
    DS18B20_GetTemp();
    Temp_Value = DS18B20_GetTemp();
    AlarmFan_Control(Temp_Value, Temp_Threshold);

    while(1) {
        Key_Process();

        if(Set_Mode == 0) {
            temp_cnt++;
            if(temp_cnt >= 10) {
                temp_cnt = 0;
                DS1302_ReadTime();
                DS1302_Time[6] = CalcWeek(DS1302_Time[0], DS1302_Time[1], DS1302_Time[2]);
                Temp_Value = DS18B20_GetTemp();
                AlarmFan_Control(Temp_Value, Temp_Threshold);
            }

            lunar_cnt++;
            if(lunar_cnt >= 10) {
                lunar_cnt = 0;
                SolarToLunar(DS1302_Time[0], DS1302_Time[1], DS1302_Time[2], &LunarTime);
            }
        }

        refresh_cnt++;
        if(refresh_cnt >= 2) {
            refresh_cnt = 0;
            Display_Refresh();
        }
        Main_DelayMs(100);
    }
}