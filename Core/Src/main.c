/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "usbd_hid.h"
#include "my_RGB.h"
#include "my_tft.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
DMA_HandleTypeDef hdma_tim1_ch2;

/* USER CODE BEGIN PV */
extern USBD_HandleTypeDef hUsbDeviceFS;
//***********************************************************************************************************************************************************
//***********************************************************************************************************************************************************
//***********************************************************************************************************************************************************
//***********************************************************************************************************************************************************
//***********************************************************************************************************************************************************
#define MyKeyColNum 11                                                                                // 定义列的数量
#define MyKeyRowNum 12                                                                                // 定义行的数量
#define MyKeyGPIOReadRow(row) HAL_GPIO_ReadPin(MyKeyRowType[row], MyKeyRowPin[row])                   // 读取某行
#define MyKeyGPIOSetCol(col) HAL_GPIO_WritePin(MyKeyColType[col], MyKeyColPin[col], GPIO_PIN_SET)     // 设置某列
#define MyKeyGPIOResetCol(col) HAL_GPIO_WritePin(MyKeyColType[col], MyKeyColPin[col], GPIO_PIN_RESET) // 重置有行

// 分配引脚
typedef GPIO_TypeDef *MyGPIOType;
MyGPIOType MyKeyColType[MyKeyColNum] = {GPIOA, GPIOA, GPIOA, GPIOA, GPIOA, GPIOA, GPIOA,
                                        GPIOC, GPIOC, GPIOB, GPIOB};
uint16_t MyKeyColPin[MyKeyColNum] = {GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3, GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7,
                                     GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_0, GPIO_PIN_1};
MyGPIOType MyKeyRowType[MyKeyRowNum] = {GPIOB, GPIOB, GPIOB, GPIOB, GPIOB, GPIOB,
                                        GPIOC, GPIOC, GPIOC, GPIOC, GPIOC, GPIOC};
uint16_t MyKeyRowPin[MyKeyRowNum] = {GPIO_PIN_10, GPIO_PIN_11, GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15,
                                     GPIO_PIN_6, GPIO_PIN_7, GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_10, GPIO_PIN_11};

#define MyKeyCountMax (200)                        // 0.1ms
int MyKeysCount[MyKeyRowNum][MyKeyColNum];         // 键盘的状态变化冷却计数器
uint8_t MyKeysLastState[MyKeyRowNum][MyKeyColNum]; // 键盘的状
uint8_t MyKeysNowState[MyKeyRowNum][MyKeyColNum];  // 键盘的状

#define DataExLen 2
uint8_t keyBoardData[16 + DataExLen] = {0x00}; // 待发送的数据0是以下按
uint16_t TFTColor = 0xF800;
uint32_t counter_01;
//|--bit0:   Left Control是否按下，按下为1
//|--bit1:   Left Shift  是否按下，按下为1
//|--bit2:   Left Alt    是否按下，按下为1
//|--bit3:   Left GUI（Windows键） 是否按下，按下为1
//|--bit4:   Right Control是否按下，按下为1
//|--bit5:   Right Shift 是否按下，按下为1
//|--bit6:   Right Alt   是否按下，按下为1
//|--bit7:   Right GUI   是否按下，按下为1

/*const uint32_t MyKeysCode[MyKeyRowNum][MyKeyColNum] = {
    //1111,222222,333333,444444,555555,666666,777777,888888,999999,101010,111111
    {'esc', '   ', '_f1', '_f2', '_f3', '_f4', '_f5', '_f6', '_f7', '_f8', '   '},//1
    {'_f9', 'f10', 'f11', 'f12', 'prt', 'scl', 'pse', '   ', '   ', '   ', '   '},//2
    //1111,222222,333333,444444,555555,666666,777777,888888,999999,101010,111111
    {'~``', '111', '222', '333', '444', '555', '666', '777', '888', '999', '000'},//3
    {'tab', 'QQQ', 'WWW', 'EEE', 'RRR', 'TTT', 'YYY', 'UUU', 'III', 'OOO', 'PPP'},//4
    {'cap', 'AAA', 'SSS', 'DDD', '   ', 'FFF', 'GGG', 'HHH', 'JJJ', 'KKK', 'LLL'},//5
    {'sft', 'ZZZ', 'XXX', 'CCC', '   ', 'VVV', 'BBB', 'NNN', 'MMM', '<,,', '>..'},//6
    {'ctl', 'win', 'alt', '   ', '   ', 'spc', '   ', '   ', '   ', '   ', 'alt'},//7
    //1111,222222,333333,444444,555555,666666,777777,888888,999999,101010,111111
    {'   ', '_--', '+==', 'bks', 'ist', 'hom', 'pgu', 'nml', '///', '***', '---'},//8
    {'   ', '{[[', '}]]', '|\\', 'del', 'end', 'pgd', 'nm7', 'nm8', 'nm9', '+++'},//9
    {'   ', ':;;', '"""', 'ent', '   ', '   ', '   ', 'nm4', 'nm5', 'nm6', '   '},//10
    {'   ', '   ', '?//', 'sft', '   ', 'upp', '   ', 'nm1', 'nm2', 'nm3', 'nme'},//11
    {'   ', 'win', 'nmu', 'ctl', 'let', 'dow', 'rig', 'nm0', '   ', '...', '   '}};//12
*/

const uint8_t MyKeysCode_Code[MyKeyRowNum][MyKeyColNum] = { // 待发送的数据地址
    // 1111,222222,333333,444444,555555,666666,777777,888888,999999,101010,111111
    {0x29, 0x00, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0x40, 0x41, 0x00}, // 1
    {0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x00, 0x00, 0x00, 0x00}, // 2
    // 1111,222222,333333,444444,555555,666666,777777,888888,999999,101010,111111
    {0x35, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27}, // 3
    {0x2b, 0x14, 0x1a, 0x08, 0x15, 0x17, 0x1c, 0x18, 0x0c, 0x12, 0x13}, // 4
    {0x39, 0x04, 0x16, 0x07, 0x00, 0x09, 0x0a, 0x0b, 0x0d, 0x0e, 0x0f}, // 5
    {0xf1, 0x1d, 0x1b, 0x06, 0x00, 0x19, 0x05, 0x11, 0x10, 0x36, 0x37}, // 6
    {0xf0, 0xf3, 0xf2, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x00, 0x00, 0xf6}, // 7
    // 1111,222222,333333,444444,555555,666666,777777,888888,999999,101010,111111
    {0x00, 0x2d, 0x2e, 0x2a, 0x49, 0x4a, 0x4b, 0x53, 0x54, 0x55, 0x56},  // 8
    {0x00, 0x2f, 0x30, 0x31, 0x4c, 0x4d, 0x4e, 0x5f, 0x60, 0x61, 0x57},  // 9
    {0x00, 0x33, 0x34, 0x28, 0x00, 0x00, 0x00, 0x5c, 0x5d, 0x5e, 0x00},  // 10
    {0x00, 0x00, 0x38, 0xf5, 0x00, 0x52, 0x00, 0x59, 0x5a, 0x5b, 0x58},  // 11
    {0x00, 0xff, 0x65, 0xf4, 0x50, 0x51, 0x4f, 0x62, 0x00, 0x63, 0x00}}; // 12
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_TIM2_Init();
  MX_USB_DEVICE_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

  //***********************************************************************************************************************************************************
  //***********************************************************************************************************************************************************
  //***********************************************************************************************************************************************************
  //***********************************************************************************************************************************************************
  //***********************************************************************************************************************************************************
  HAL_TIM_Base_Start_IT(&htim2);        // 重启定时
  for (int i = 0; i < MyKeyColNum; i++) // 全部置为0
  {
    MyKeyGPIOResetCol(i);
  }
  TFT_SyncInit(GPIOB, GPIO_PIN_4,
               GPIOB, GPIO_PIN_5,
               GPIOB, GPIO_PIN_6,
               GPIOB, GPIO_PIN_7,
               GPIOB, GPIO_PIN_8,
               GPIOB, GPIO_PIN_9);

  MyTFT_SyncPrintChar('O', 0xff00);
  MyTFT_SyncPrintChar('K', 0x00ff);
  MyTFT_CloseScreen();

  RGB_SetAllColor32(RGB_Color_BLACK);
  RGB_SendLEDData();
  // HAL_TIM_Base_Start_IT(&htim3); // 初始化后再重启定�????

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    //***********************************************************************************************************************************************************
    //***********************************************************************************************************************************************************
    //***********************************************************************************************************************************************************
    //***********************************************************************************************************************************************************
    //***********************************************************************************************************************************************************
    uint32_t keycode = 0;
    uint32_t showCodeNumber = keycode;
    static uint32_t lastkeycode = 0;
    for (int i = 0; i < sizeof(keyBoardData); i++)
    {
      keyBoardData[i] = 0;
    }
    int num = 0;
    char fnIsDown = 0;
    for (int i = 0; i < MyKeyRowNum; i++)
    {
      for (int j = 0; j < MyKeyColNum; j++)
      {
        if (MyKeysNowState[i][j])
        {
          if ((MyKeysCode_Code[i][j] & 0xf0) == 0xf0)
          {
            if ((MyKeysCode_Code[i][j] == 0xff))
            {
              fnIsDown = 1; // fnfnfn
            }
            else
            {
              keyBoardData[0] |= (1 << (MyKeysCode_Code[i][j] & 0x0f));//特殊按键
            }
          }
          else
          {
            keyBoardData[(MyKeysCode_Code[i][j] / 8) + 1] |= (1 << (MyKeysCode_Code[i][j] % 8));//普通按键
          }
          keycode = MyKeysCode_Code[i][j];
          num++;
        }
      }
    }

    static char state = 0;
    static char RCode = 0;
    static char GCode = 0;
    static char BCode = 0;
    static char nowCode = 0;
    if (fnIsDown)
    {
      nowCode = 0;
      if (keyBoardData[(0x52 / 8) + 1] & (1 << (0x52 % 8))) // up
      {
        state = 1;
      }
      if (keyBoardData[(0x51 / 8) + 1] & (1 << (0x51 % 8))) // down
      {
        state = 0;
      }
      if (keyBoardData[(0x50 / 8) + 1] & (1 << (0x50 % 8))) // left
      {
        MyTFT_CloseScreen();
      }
      if (keyBoardData[(0x4f / 8) + 1] & (1 << (0x4f % 8))) // right
      {
        MyTFT_OpenScreen();
      }

      if (keyBoardData[(0x62 / 8) + 1] & (1 << (0x62 % 8))) //0
      {
        // state = 0;
      }
      if (keyBoardData[(0x59 / 8) + 1] & (1 << (0x59 % 8))) //1
      {
        nowCode|=0x01;
        // state = 0x59;
      }
      if (keyBoardData[(0x5a / 8) + 1] & (1 << (0x5a % 8))) //2
      {
        nowCode|=0x02;
        // state = 0x5a;
      }
      if (keyBoardData[(0x5b / 8) + 1] & (1 << (0x5b % 8))) //3
      {
        nowCode|=0x04;
        // state = 0x5b;
      }
      if (keyBoardData[(0x5c / 8) + 1] & (1 << (0x5c % 8))) //4
      {
        nowCode|=0x08;
        // state = 0x5c;
      }
      if (keyBoardData[(0x5d / 8) + 1] & (1 << (0x5d % 8))) //5
      {
        nowCode|=0x10;
        // state = 0x5d;
      }
      if (keyBoardData[(0x5e / 8) + 1] & (1 << (0x5e % 8))) //6
      {
        nowCode|=0x20;
        // state = 0x5e;
      }
      if (keyBoardData[(0x5f / 8) + 1] & (1 << (0x5f % 8))) //7
      {
        nowCode|=0x40;
        // state = 0x5f;
      }
      if (keyBoardData[(0x60 / 8) + 1] & (1 << (0x60 % 8))) //8
      {
        nowCode|=0x80;
        // state = 0x60;
      }
      if (keyBoardData[(0x61 / 8) + 1] & (1 << (0x61 % 8))) //9
      {
        // state = 0x61;
      }
      if (keyBoardData[(0x15 / 8) + 1] & (1 << (0x15 % 8))) //r
      {
        RCode=nowCode;
        // state = 0x61;
      }
      if (keyBoardData[(0x0a / 8) + 1] & (1 << (0x0a % 8))) //g
      {
        GCode=nowCode;
        // state = 0x61;
      }
      if (keyBoardData[(0x05 / 8) + 1] & (1 << (0x05 % 8))) //b
      {
        BCode=nowCode;
        // state = 0x61;
      }

      ////////
      if (keyBoardData[(0x3a / 8) + 1] & (1 << (0x3a % 8))) // f1
      {
        keyBoardData[sizeof(keyBoardData) - DataExLen] = 0x80;
      }

      if (keyBoardData[(0x3b / 8) + 1] & (1 << (0x3b % 8))) // f2
      {
        keyBoardData[sizeof(keyBoardData) - DataExLen]= 0x81;
      }
      if (keyBoardData[(0x3c / 8) + 1] & (1 << (0x3c % 8))) // f3
      {
        keyBoardData[sizeof(keyBoardData) - DataExLen]= 0x82;
      }
      if (keyBoardData[(0x3d / 8) + 1] & (1 << (0x3d % 8))) // f4
      {
        keyBoardData[sizeof(keyBoardData) - DataExLen]= 0x83;
      }
      if (keyBoardData[(0x3e / 8) + 1] & (1 << (0x3e % 8))) // f5
      {
        keyBoardData[sizeof(keyBoardData) - DataExLen]= 0x84;
      }
      if (keyBoardData[(0x3f / 8) + 1] & (1 << (0x3f % 8))) // f6
      {
        keyBoardData[sizeof(keyBoardData) - DataExLen]= 0x85;
      }
      if (keyBoardData[(0x40 / 8) + 1] & (1 << (0x40 % 8))) // f7
      {
        keyBoardData[sizeof(keyBoardData) - DataExLen]= 0x86;
      }
      if (keyBoardData[(0x41 / 8) + 1] & (1 << (0x41 % 8))) // f8
      {
        keyBoardData[sizeof(keyBoardData) - DataExLen]= 0x87;
      }

      
      if (keyBoardData[(0x42 / 8) + 1] & (1 << (0x42 % 8))) // f9
      {
        keyBoardData[sizeof(keyBoardData) - DataExLen]= 90; //vol
      }
      if (keyBoardData[(0x43 / 8) + 1] & (1 << (0x43 % 8))) // f10
      {
        keyBoardData[sizeof(keyBoardData) - DataExLen]= 100; //vol
      }
      if (keyBoardData[(0x44 / 8) + 1] & (1 << (0x44 % 8))) // f11
      {
        keyBoardData[sizeof(keyBoardData) - DataExLen]= 00; //vol
      }
      if (keyBoardData[(0x45 / 8) + 1] & (1 << (0x45 % 8))) // f12
      {
        keyBoardData[sizeof(keyBoardData) - DataExLen]= 101; //vol
      }

      for (int i = 0; i < sizeof(keyBoardData) - DataExLen; i++) // fn set zero
      {
        keyBoardData[i] = 0;
      }
    }

    if (counter_01 > 10 * 200)//200ms
    {
      counter_01 = 0;
      /*
      switch (state)
      {
      case 0:
        RGB_SetAllColor32(RGB_Color(0, 0, 0));
        //RGB_SendLEDData();
        break;

      case 1:
        RGB_SetAllColor32(RGB_Color(3, 3, 3));
        //RGB_SendLEDData();
        state++;
        break;
      case 2:
        RGB_SetAllColor32(RGB_Color(4, 4, 4));
        //RGB_SendLEDData();
        state++;
        break;
      case 3:
        RGB_SetAllColor32(RGB_Color(5, 5, 5));
        //RGB_SendLEDData();
        state++;
        break;
      case 4:
        RGB_SetAllColor32(RGB_Color(4, 4, 4));
        //RGB_SendLEDData();
        state++;
        break;
      case 5:
        RGB_SetAllColor32(RGB_Color(3, 3, 3));
        //RGB_SendLEDData();
        state = 1;
        break;

      case 0x59: // 1
        RGB_SetAllColor32(RGB_Color(3, 3, 3));
        //RGB_SendLEDData();
        break;
      case 0x5a: // 2
        RGB_SetAllColor32(RGB_Color(4, 4, 4));
        //RGB_SendLEDData();
        break;
      case 0x5b: // 3
        RGB_SetAllColor32(RGB_Color(5, 5, 5));
        //RGB_SendLEDData();
        break;
      case 0x5c: // 4
        RGB_SetAllColor32(RGB_Color(6, 6, 6));
        //RGB_SendLEDData();
        break;
      case 0x5d: // 5
        RGB_SetAllColor32(RGB_Color(7, 7, 7));
        //RGB_SendLEDData();
        break;
      case 0x5e: // 6
        RGB_SetAllColor32(RGB_Color(8, 8, 8));
        //RGB_SendLEDData();
        break;
      case 0x5f: // 7
        RGB_SetAllColor32(RGB_Color(9, 9, 9));
        //RGB_SendLEDData();
        break;
      case 0x60: // 8
        RGB_SetAllColor32(RGB_Color(10, 10, 10));
        //RGB_SendLEDData();
        break;
      case 0x61: // 9
        RGB_SetAllColor32(RGB_Color(11, 11, 11));
        //RGB_SendLEDData();
        break;

      default:
        state = 0;
        break;
      }*/
      
      RGB_SetAllColor32(RGB_Color(RCode, GCode, BCode));
      if (fnIsDown)
      {
        RGB_SendLEDData();
      }
    }
    if (lastkeycode != keycode)
    {
      switch (TFTColor)
      {
      case 0xF800:
        TFTColor = 0x07E0;
        break;
      case 0x07E0:
        TFTColor = 0x001F;
        break;
      case 0x001F:
        TFTColor = 0XFFE0;
        break;
      case 0XFFE0:
        TFTColor = 0XF81F;
        break;
      case 0XF81F:
        TFTColor = 0X07FF;
        break;
      case 0X07FF:
        TFTColor = 0xF800;
        break;
      default:
        TFTColor = 0xF800;
        break;
      }
      switch (keycode)
      {
      case 0:
        // MyTFT_SyncPrintChar('8', 0x0000);
        // RGB_SetAllColor32(RGB_Color_BLACK);
        // RGB_SendLEDData();
        break;
      default:
        showCodeNumber = keycode;
        if(showCodeNumber>=0x04 && showCodeNumber<=0x1D){
          showCodeNumber += 61;
          MyTFT_SyncPrintChar(showCodeNumber, TFTColor);
          // showCodeNumber += 32;
        }
        else if (showCodeNumber == 0x2C)
        {
          MyTFT_SyncPrintChar(' ', TFTColor);
        }
        else if (showCodeNumber == 0x1E)
        {
          MyTFT_SyncPrintChar('!', TFTColor);
        }
        
        
        // MyTFT_SyncPrintChar(num + '0', 0x00ff);
        // RGB_SetAllColor32(RGB_Color(2, 2, 2));
        // RGB_SetColor32(3, RGB_Color_BLUE);
        // RGB_SendLEDData();
        break;
      }
    }

    lastkeycode = keycode;
    USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t *)&keyBoardData, sizeof(keyBoardData));
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * @brief TIM1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 89;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);
}

/**
 * @brief TIM2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 72 - 1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 100 - 1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
}

/**
 * @brief TIM3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 71;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
}

/**
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4 | GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA1 PA2 PA3 PA4
                           PA5 PA6 PA7 PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PC4 PC5 */
  GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB4 PB5
                           PB6 PB7 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB10 PB11 PB12 PB13
                           PB14 PB15 */
  GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PC6 PC7 PC8 PC9
                           PC10 PC11 */
  GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
//***********************************************************************************************************************************************************
//***********************************************************************************************************************************************************
//***********************************************************************************************************************************************************
//***********************************************************************************************************************************************************
//***********************************************************************************************************************************************************
int myNowCol = 0; // 当前1的列
// int num = 0;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2) // 0.1ms 每次中断，换个列进行扫描，大0.1ms
  {
    counter_01++;
    MyTFT_SyncCore();
    for (int i = 0; i < MyKeyRowNum; i++) // 读取有的
    {
      if (MyKeysCount[i][myNowCol] > 0) //
      {
        MyKeysCount[i][myNowCol] -= MyKeyColNum;
      }
      else //
      {
        if (MyKeyGPIOReadRow(i) == GPIO_PIN_SET) // 如果当前的行读取1
        {
          MyKeysNowState[i][myNowCol] = 1;
        }
        else
        {
          MyKeysNowState[i][myNowCol] = 0;
        }
        if (MyKeysLastState[i][myNowCol] != MyKeysNowState[i][myNowCol]) // 按键相比于之前发生了变化，进入冷
        {
          MyKeysCount[i][myNowCol] = MyKeyCountMax;
          MyKeysLastState[i][myNowCol] = MyKeysNowState[i][myNowCol];
        }
      }
    }
    // 换到下一个引
    MyKeyGPIOResetCol(myNowCol);
    myNowCol++;
    myNowCol %= MyKeyColNum;
    MyKeyGPIOSetCol(myNowCol);
  }
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
