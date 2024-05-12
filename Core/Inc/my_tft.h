#ifndef _MY_TFT_H
#define _MY_TFT_H

#include "main.h"

#define MyTFTOrigW 4
#define MyTFTOrigH 4
#define MyTFTMaxW 128
#define MyTFTMaxH 128
#define MyTFTDeltax 8;
#define MyTFTDeltay 16;

#define USE_HORIZONTAL 3 // 设置横屏或者竖屏显示 0或1为竖屏 2或3为横屏

#define TFT_RGB(r, g, b) ((uint16_t)(((uint16_t)r << 11) & 0xf800) | \
                          (uint16_t)(((uint16_t)g << 5) & 0x07e0) |  \
                          (uint16_t)((uint16_t)b & 0x1f)) // R32G64B32
#ifdef NoSync
void TFT_Init(GPIO_TypeDef *type_SCL, uint16_t pin_SCL,
              GPIO_TypeDef *type_SDA, uint16_t pin_SDA,
              GPIO_TypeDef *type_RST, uint16_t pin_RST,
              GPIO_TypeDef *type_DC, uint16_t pin_DC,
              GPIO_TypeDef *type_CS, uint16_t pin_CS,
              GPIO_TypeDef *type_BLK, uint16_t pin_BLK);

void MyTFT_WriteData(int x, int y, int len, int wid, uint16_t *colors);
void MyTFT_WriteASCII(int x, int y, uint8_t ascii, uint16_t color, uint16_t bgcolor);
void MyTFT_PrintChar(char code, uint16_t color);
#endif

#define SYNC_BUF_LEN 16384

void TFT_SyncInit(GPIO_TypeDef *type_SCL, uint16_t pin_SCL,
                  GPIO_TypeDef *type_SDA, uint16_t pin_SDA,
                  GPIO_TypeDef *type_RST, uint16_t pin_RST,
                  GPIO_TypeDef *type_DC, uint16_t pin_DC,
                  GPIO_TypeDef *type_CS, uint16_t pin_CS,
                  GPIO_TypeDef *type_BLK, uint16_t pin_BLK);

void MyTFT_SyncCore();//中断调用这个
void MyTFT_SyncWriteData(int x, int y, int len, int wid, uint16_t *colors);
void MyTFT_SyncWriteASCII(int x, int y, uint8_t ascii, uint16_t color, uint16_t bgcolor);
void MyTFT_SyncPrintChar(char code, uint16_t color);

void MyTFT_OpenScreen();
void MyTFT_CloseScreen();
#endif


