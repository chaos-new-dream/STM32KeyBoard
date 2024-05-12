#ifndef _MY_RGB_H
#define _MY_RGB_H

#include "main.h"

#define RGB_Color(r, g, b) (((g & 0xff) << 16) | ((r & 0xff) << 8) | (b & 0xff))
#define RGB_Color_RED RGB_Color(255, 0, 0) // 显示红色RGB数据
#define RGB_Color_GREEN RGB_Color(0, 255, 0)
#define RGB_Color_BLUE RGB_Color(0, 0, 255)
#define RGB_Color_SKY RGB_Color(0, 255, 255)
#define RGB_Color_MAGENTA RGB_Color(255, 0, 220)
#define RGB_Color_YELLOW RGB_Color(127, 216, 0)
#define RGB_Color_OEANGE RGB_Color(127, 106, 0)
#define RGB_Color_BLACK RGB_Color(0, 0, 0)
#define RGB_Color_WHITE RGB_Color(255, 255, 255)
/*建立一个定义单个LED三原色值大小的结构体*/

#define LED_NUM 104 // LED数量宏定义，这里我使用一个LED，（单词pixel为像素的意思）

void RGB_SetColor32(int LedId, uint32_t agrb);                    // 给一个LED装载24个颜色数据码（0码和1码）
void RGB_SetColor888(int LedId, uint8_t r, uint8_t g, uint8_t b); // 给一个LED装载24个颜色数据码（0码和1码）
void RGB_SetAllColor32(uint32_t color);
void RGB_SetAllColor888(uint8_t r, uint8_t g, uint8_t b);
void RGB_PushColor(uint32_t agrb);
void RGB_SendLEDData();                                           // 发送最终数组

#endif


