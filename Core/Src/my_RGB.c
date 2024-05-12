
#include "my_RGB.h"

#define CODE_1 (60) // 1码定时器计数次数
#define CODE_0 (23) // 0码定时器计数次数
#define ZeroHead 50
#define BufLen (ZeroHead + (LED_NUM + 1) * 24)
/*Some Static Colors------------------------------*/

extern TIM_HandleTypeDef htim1;
/*二维数组存放最终PWM输出数组，每一行24个
数据代表一个LED，最后一行24个0代表RESET码*/
uint32_t LED_Buf[BufLen];
uint32_t Color_Buf[LED_NUM];
/*
功能：设定单个RGB LED的颜色，把结构体中RGB的24BIT转换为0码和1码
参数：LedId为LED序号，Color：定义的颜色结构体
*/
void RGB_SetColor32(int LedId, uint32_t agrb)
{
    if (LedId > LED_NUM)
        return; // avoid overflow 防止写入ID大于LED总数
    Color_Buf[LedId] = agrb;
    for (int i = 0; i < 24; i++)
    {
        LED_Buf[ZeroHead + LedId * 24 + i] = ((agrb & (0x800000 >> i)) ? (CODE_1) : CODE_0);
    }
}
void RGB_SetColor888(int LedId, uint8_t r, uint8_t g, uint8_t b)
{
    RGB_SetColor32(LedId, RGB_Color(r, g, b));
}
/*
功能：最后一行装在24个0，输出24个周期占空比为0的PWM波，作为最后reset延时，这里总时长为24*1.2=30us > 24us(要求大于24us)
*/
void Reset_Load()
{
    uint8_t i;
    for (i = 0; i < ZeroHead; i++)
    {
        LED_Buf[i] = 0;
    }
    for (i = 0; i < 24; i++)
    {
        LED_Buf[ZeroHead + LED_NUM * 24 + i] = 0;
    }
}

/*
功能：发送数组
参数：(&htim1)定时器1，(TIM_CHANNEL_1)通道1，((uint32_t *)Pixel_Buf)待发送数组，
            (Pixel_NUM+1)*24)发送个数，数组行列相乘
*/
void RGB_SendLEDData()
{
    Reset_Load();
    HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_2, (uint32_t *)LED_Buf, BufLen);
    // HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, (uint32_t *)Pixel_Buf, (Pixel_NUM + 1) * 24);
}

void RGB_SetAllColor32(uint32_t color)
{
    for (int i = 0; i < LED_NUM; i++)
    {
        RGB_SetColor32(i, color);
    }
}
void RGB_SetAllColor888(uint8_t r, uint8_t g, uint8_t b)
{
    RGB_SetAllColor32(RGB_Color(r, g, b));
}

void RGB_PushColor(uint32_t agrb)
{
    for (int i = LED_NUM-1; i >0; i--)
    {
        Color_Buf[i] = Color_Buf[i - 1];
        RGB_SetColor32(i, Color_Buf[i]);
    }
    Color_Buf[0] = agrb;
    RGB_SetColor32(0, agrb);
}
