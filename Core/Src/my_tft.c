#include "my_tft.h"

#include "lcdfont.h"
#define WHITE 0xFFFF
#define BLACK 0x0000
#define BLUE 0x001F
#define BRED 0XF81F
#define GRED 0XFFE0
#define GBLUE 0X07FF
#define RED 0xF800
#define MAGENTA 0xF81F
#define GREEN 0x07E0
#define CYAN 0x7FFF
#define YELLOW 0xFFE0
#define BROWN 0XBC40	  // 棕色
#define BRRED 0XFC07	  // 棕红色
#define GRAY 0X8430		  // 灰色
#define DARKBLUE 0X01CF	  // 深蓝色
#define LIGHTBLUE 0X7D7C  // 浅蓝色
#define GRAYBLUE 0X5458	  // 灰蓝色
#define LIGHTGREEN 0X841F // 浅绿色
#define LGRAY 0XC618	  // 浅灰色(PANNEL),窗体背景色
#define LGRAYBLUE 0XA651  // 浅灰蓝色(中间层颜色)
#define LBBLUE 0X2B12	  // 浅棕蓝色(选择条目的反色)

GPIO_TypeDef *GPIOx_SCL_nowUsing;
uint16_t GPIO_Pin_SCL_nowUsing;
#define IO_SCLK_CLR0 HAL_GPIO_WritePin(GPIOx_SCL_nowUsing, GPIO_Pin_SCL_nowUsing, GPIO_PIN_RESET)
#define IO_SCLK_SET1 HAL_GPIO_WritePin(GPIOx_SCL_nowUsing, GPIO_Pin_SCL_nowUsing, GPIO_PIN_SET)

GPIO_TypeDef *GPIOx_SDA_nowUsing;
uint16_t GPIO_Pin_SDA_nowUsing;
#define IO_SDAT_CLR0 HAL_GPIO_WritePin(GPIOx_SDA_nowUsing, GPIO_Pin_SDA_nowUsing, GPIO_PIN_RESET)
#define IO_SDAT_SET1 HAL_GPIO_WritePin(GPIOx_SDA_nowUsing, GPIO_Pin_SDA_nowUsing, GPIO_PIN_SET)

// 低电平复位
GPIO_TypeDef *GPIOx_RST_nowUsing;
uint16_t GPIO_Pin_RST_nowUsing;
#define IO_RST0_StartRST HAL_GPIO_WritePin(GPIOx_RST_nowUsing, GPIO_Pin_RST_nowUsing, GPIO_PIN_RESET)
#define IO_RST1_EndRST HAL_GPIO_WritePin(GPIOx_RST_nowUsing, GPIO_Pin_RST_nowUsing, GPIO_PIN_SET)

// 0指令1数据
GPIO_TypeDef *GPIOx_DC_nowUsing;
uint16_t GPIO_Pin_DC_nowUsing;
#define IO_DC_CMD0 HAL_GPIO_WritePin(GPIOx_DC_nowUsing, GPIO_Pin_DC_nowUsing, GPIO_PIN_RESET)
#define IO_DC_DAT1 HAL_GPIO_WritePin(GPIOx_DC_nowUsing, GPIO_Pin_DC_nowUsing, GPIO_PIN_SET)

// 片选0写入
GPIO_TypeDef *GPIOx_CS_nowUsing;
uint16_t GPIO_Pin_CS_nowUsing;
#define IO_CS0_Start HAL_GPIO_WritePin(GPIOx_CS_nowUsing, GPIO_Pin_CS_nowUsing, GPIO_PIN_RESET) // 片选
#define IO_CS1_End HAL_GPIO_WritePin(GPIOx_CS_nowUsing, GPIO_Pin_CS_nowUsing, GPIO_PIN_SET)		// 片不选

// 高电平背光
GPIO_TypeDef *GPIOx_BLK_nowUsing;
uint16_t GPIO_Pin_BLK_nowUsing;
#define IO_BLK0_CLose HAL_GPIO_WritePin(GPIOx_BLK_nowUsing, GPIO_Pin_BLK_nowUsing, GPIO_PIN_RESET)
#define IO_BLK1_Open HAL_GPIO_WritePin(GPIOx_BLK_nowUsing, GPIO_Pin_BLK_nowUsing, GPIO_PIN_SET)

// 放置字符串的时候会用到
int mytft_nowx = MyTFTOrigW;
int mytft_nowy = MyTFTOrigH;

uint8_t syncDataBuffer[SYNC_BUF_LEN];	  // 待传输数据
uint8_t syncDataTypeBuffer[SYNC_BUF_LEN]; // 0指令1数据，2设置RST并延时，3延时并结束RST，4单纯延时
uint8_t syncPerBitNowState = 0;			  // 当前状态：0未解析1要设置电平2时钟下调3时钟上调
int syncNowIndex = 0;					  // 当前待传输的数据索引
uint8_t syncNowBit = 0x80;				  // 当前在传的位
int syncTotalLast = 0;					  // 剩余待传输的数据数量
// 增加一个待传输的数据0指令1数据2重置2重置结束
void __AddData(uint8_t dataType, uint8_t data)
{
	int newIndex = (syncNowIndex + syncTotalLast) % SYNC_BUF_LEN;
	syncDataTypeBuffer[newIndex] = dataType;
	syncDataBuffer[newIndex] = data;
	syncTotalLast++;
}
#define AddDelay       \
	__AddData(4, 250); \
	__AddData(4, 250); \
	__AddData(4, 250); \
	__AddData(4, 250); \
	__AddData(4, 250)
#define AddReset       \
	__AddData(2, 250); \
	__AddData(2, 250); \
	__AddData(2, 250); \
	__AddData(2, 250); \
	__AddData(3, 250)
#define AddData16(d16)      \
	__AddData(1, (d16) >> 8); \
	__AddData(1, (d16))
#define AddData8(d8) __AddData(1, (d8))
#define AddCommand(d8) __AddData(0, (d8))
// 进行下一个数据的传输
void NextData()
{
	if (syncTotalLast)
	{
		syncNowIndex++;
		syncNowIndex %= SYNC_BUF_LEN;
		syncPerBitNowState = 0;
		syncTotalLast--;
	}
}

void TFT_SyncInit(GPIO_TypeDef *type_SCL, uint16_t pin_SCL,
				  GPIO_TypeDef *type_SDA, uint16_t pin_SDA,
				  GPIO_TypeDef *type_RST, uint16_t pin_RST,
				  GPIO_TypeDef *type_DC, uint16_t pin_DC,
				  GPIO_TypeDef *type_CS, uint16_t pin_CS,
				  GPIO_TypeDef *type_BLK, uint16_t pin_BLK)
{
	GPIOx_SCL_nowUsing = type_SCL;
	GPIO_Pin_SCL_nowUsing = pin_SCL;
	IO_SCLK_CLR0;
	GPIOx_SDA_nowUsing = type_SDA;
	GPIO_Pin_SDA_nowUsing = pin_SDA;
	IO_SDAT_SET1;
	GPIOx_DC_nowUsing = type_DC;
	GPIO_Pin_DC_nowUsing = pin_DC;
	IO_DC_DAT1;
	GPIOx_CS_nowUsing = type_CS;
	GPIO_Pin_CS_nowUsing = pin_CS;
	IO_CS1_End;
	GPIOx_BLK_nowUsing = type_BLK;
	GPIO_Pin_BLK_nowUsing = pin_BLK;
	IO_BLK1_Open;
	GPIOx_RST_nowUsing = type_RST;
	GPIO_Pin_RST_nowUsing = pin_RST;
	IO_RST0_StartRST;

	AddReset;
	AddCommand(0x11);
	AddDelay;
	AddCommand(0x36); // 设置内存扫描方向，0X00正常扫描，从上往下，从左往右，RGB方式

	if (USE_HORIZONTAL == 0)
		AddData8(0x00);
	else if (USE_HORIZONTAL == 1)
		AddData8(0xC0);
	else if (USE_HORIZONTAL == 2)
		AddData8(0x70);
	else
		AddData8(0xA0);

	AddCommand(0x3A); // 数据格式，65K色,565
	AddData8(0x05);

	AddCommand(0xB2); // 帧频设置
	AddData8(0x0C);
	AddData8(0x0C);
	AddData8(0x00);
	AddData8(0x33);
	AddData8(0x33);

	AddCommand(0xB7); // GATE 设置
	AddData8(0x35);

	AddCommand(0xBB); // VCOM设置
	AddData8(0x19);

	AddCommand(0xC0); // LCM设置,默认0x2c
	AddData8(0x2C);

	AddCommand(0xC2); // VDV&VRH SET ,默认0x01
	AddData8(0x01);

	AddCommand(0xC3); // VRHS SET，默认0x0b
	AddData8(0x12);	  // 此处根据实际情况修正

	AddCommand(0xC4); // VDV SET，默认0x20
	AddData8(0x20);

	AddCommand(0xC6); // FR SET, 默认0x0F
	AddData8(0x0F);

	AddCommand(0xD0); // 电源控制1
	AddData8(0xA4);	  // 该参数不变
	AddData8(0xA1);	  // 此处根据实际情况修改

	AddCommand(0xE0); // 正极性GAMMA调整
	AddData8(0xD0);
	AddData8(0x04);
	AddData8(0x0D);
	AddData8(0x11);
	AddData8(0x13);
	AddData8(0x2B);
	AddData8(0x3F);
	AddData8(0x54);
	AddData8(0x4C);
	AddData8(0x18);
	AddData8(0x0D);
	AddData8(0x0B);
	AddData8(0x1F);
	AddData8(0x23);

	AddCommand(0xE1); // 负极性GAMMA调整
	AddData8(0xD0);
	AddData8(0x04);
	AddData8(0x0C);
	AddData8(0x11);
	AddData8(0x13);
	AddData8(0x2C);
	AddData8(0x3F);
	AddData8(0x44);
	AddData8(0x51);
	AddData8(0x2F);
	AddData8(0x1F);
	AddData8(0x1F);
	AddData8(0x20);
	AddData8(0x23);

	// AddCommand(0x21); // 反显开，默认是0X20，正常模式

	AddCommand(0x29);
}
void MyTFT_SyncCore()
{
	if (syncTotalLast > 0)
	{
		switch (syncPerBitNowState)
		{
		case 0: // 未解析

			switch (syncDataTypeBuffer[syncNowIndex]) // 进行解析
			{
			case 0: // 指令
				IO_DC_CMD0;
				IO_RST1_EndRST;
				IO_CS0_Start;
				syncNowBit = 0x80;
				syncPerBitNowState = 1;
				break;
			case 1: // 数据
				IO_DC_DAT1;
				IO_RST1_EndRST;
				IO_CS0_Start;
				syncNowBit = 0x80;
				syncPerBitNowState = 1;
				break;
			case 2: // 延时并进行Reset
				IO_RST0_StartRST;
				if (syncDataBuffer[syncNowIndex])
				{
					syncDataBuffer[syncNowIndex]--;
				}
				else
				{
					NextData();
					return;
				}
				break;
			case 3: // 延时并重置Reset
				if (syncDataBuffer[syncNowIndex])
				{
					syncDataBuffer[syncNowIndex]--;
				}
				else
				{
					IO_RST1_EndRST;
					NextData();
					return;
				}
				return;
				break;
			case 4: // 单纯延时
				if (syncDataBuffer[syncNowIndex])
				{
					syncDataBuffer[syncNowIndex]--;
				}
				else
				{
					NextData();
					return;
				}
				break;
			}
			break;

		case 1: // 设置电平

			if (syncNowBit) // 还有要传的数据
			{
				syncPerBitNowState = 2;
				if (syncDataBuffer[syncNowIndex] & syncNowBit)
				{
					IO_SDAT_SET1;
				}
				else
				{
					IO_SDAT_CLR0;
				}
				syncNowBit >>= 1;
			}
			else
			{
				IO_CS1_End; // 片选结束
				NextData();
				return;
			}
			break;

		case 2: // 时钟上升
			syncPerBitNowState = 3;
			IO_SCLK_SET1;
			break;
		case 3: // 时钟下降
			syncPerBitNowState = 1;
			IO_SCLK_CLR0;
			break;

		default:
			break;
		}
	}
}

void MyTFT_SyncWriteData(int x, int y, int len, int wid, uint16_t *colors)
{
	int startX = 0, startY = 0;
	len--;
	wid--;
	if (USE_HORIZONTAL == 0)
	{
		startX = 52;
		startY = 40;
	}
	else if (USE_HORIZONTAL == 1)
	{
		startX = 53;
		startY = 40;
	}
	else if (USE_HORIZONTAL == 2)
	{
		startX = 40;
		startY = 53;
	}
	else
	{
		startX = 40;
		startY = 50;
	}
	startX = 0, startY = 0;
	AddCommand(0x2a); // 列地址设置
	AddData16(x + startX);
	AddData16(x + len + startX);
	AddCommand(0x2b); // 行地址设置
	AddData16(y + startY);
	AddData16(y + wid + startY);
	AddCommand(0x2c); // 储存器写
	for (int i = 0; i < len * wid; i++)
	{
		AddData16(colors[i]);
	}
}
uint16_t datas[128];
void MyTFT_SyncWriteASCII(int x, int y, uint8_t ascii, uint16_t color, uint16_t bgcolor)
{
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if (ascii_1608[ascii - 0x20][i] & (1 << j))
			{
				datas[i * 8 + j] = color;
			}
			else
				datas[i * 8 + j] = bgcolor;
		}
	}
	MyTFT_SyncWriteData(x, y, 8, 16, datas);
}
void MyTFT_SyncPrintChar(char code, uint16_t color)
{
	MyTFT_SyncWriteASCII(mytft_nowx, mytft_nowy, code, color, ~color);
	mytft_nowx += MyTFTDeltax;
	if (mytft_nowx > MyTFTMaxW)
	{
		mytft_nowx = MyTFTOrigW;
		mytft_nowy += MyTFTDeltay;
		if (mytft_nowy > MyTFTMaxH)
		{
			mytft_nowy = MyTFTOrigH;
		}
	}
	MyTFT_SyncWriteASCII(mytft_nowx, mytft_nowy, '#', color, ~color);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////now//////////////////////////////////////////////////////////////////////////////////////////////////
////////***//***/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef NoSync
#define this_____Nop \
	{                \
		int i = 50;  \
		while (i--)  \
			;        \
	}
void SendByte(uint8_t byte)
{
	IO_CS0_Start; // 片选
	uint8_t code = 0x80;
	while (code)
	{
		IO_SCLK_CLR0;
		if (byte & code)
			IO_SDAT_SET1;
		else
			IO_SDAT_CLR0;
		this_____Nop;
		IO_SCLK_SET1;
		this_____Nop;
		IO_SCLK_CLR0;
		this_____Nop;
		code >>= 1;
	}
	IO_CS1_End; // 片选
	this_____Nop;
}
void SendCommand(uint8_t byte)
{
	IO_DC_CMD0; // 写指令
	SendByte(byte);
}
void SendDATA8(uint8_t byte)
{
	IO_DC_DAT1; // 写数据
	SendByte(byte);
}
void SendDATA16(uint16_t word)
{
	IO_DC_DAT1; // 写数据
	SendByte(word >> 8);
	SendByte(word);
}

void MyTFT_WriteData(int x, int y, int len, int wid, uint16_t *colors)
{
	int startX = 0, startY = 0;
	len--;
	wid--;
	if (USE_HORIZONTAL == 0)
	{
		startX = 52;
		startY = 40;
	}
	else if (USE_HORIZONTAL == 1)
	{
		startX = 53;
		startY = 40;
	}
	else if (USE_HORIZONTAL == 2)
	{
		startX = 40;
		startY = 53;
	}
	else
	{
		startX = 40;
		startY = 50;
	}
	startX = 0, startY = 0;
	SendCommand(0x2a); // 列地址设置
	SendDATA16(x + startX);
	SendDATA16(x + len + startX);
	SendCommand(0x2b); // 行地址设置
	SendDATA16(y + startY);
	SendDATA16(y + wid + startY);
	SendCommand(0x2c); // 储存器写
	for (int i = 0; i < len * wid; i++)
	{
		SendDATA16(colors[i]);
	}
}

void MyTFT_PrintChar(char code, uint16_t color)
{
	MyTFT_WriteASCII(mytft_nowx, mytft_nowy, code, color, ~color);
	mytft_nowx += MyTFTDeltax;
	if (mytft_nowx > MyTFTMaxW)
	{
		mytft_nowx = MyTFTOrigW;
		mytft_nowy += MyTFTDeltay;
		if (mytft_nowy > MyTFTMaxH)
		{
			mytft_nowy = MyTFTOrigH;
		}
	}
	MyTFT_WriteASCII(mytft_nowx, mytft_nowy, '#', color, ~color);
}
uint16_t datas[128];
void MyTFT_WriteASCII(int x, int y, uint8_t ascii, uint16_t color, uint16_t bgcolor)
{
	for (int i = 0; i < 16; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			if (ascii_1608[ascii - 0x20][i] & (1 << j))
			{
				datas[i * 8 + j] = color;
			}
			else
				datas[i * 8 + j] = bgcolor;
		}
	}
	MyTFT_WriteData(x, y, 8, 16, datas);
}
void TFT_Init(GPIO_TypeDef *type_SCL, uint16_t pin_SCL,
			  GPIO_TypeDef *type_SDA, uint16_t pin_SDA,
			  GPIO_TypeDef *type_RST, uint16_t pin_RST,
			  GPIO_TypeDef *type_DC, uint16_t pin_DC,
			  GPIO_TypeDef *type_CS, uint16_t pin_CS,
			  GPIO_TypeDef *type_BLK, uint16_t pin_BLK)
{
	GPIOx_SCL_nowUsing = type_SCL;
	GPIO_Pin_SCL_nowUsing = pin_SCL;
	IO_SCLK_CLR0;
	GPIOx_SDA_nowUsing = type_SDA;
	GPIO_Pin_SDA_nowUsing = pin_SDA;
	IO_SDAT_SET1;
	GPIOx_DC_nowUsing = type_DC;
	GPIO_Pin_DC_nowUsing = pin_DC;
	IO_DC_DAT1;
	GPIOx_CS_nowUsing = type_CS;
	GPIO_Pin_CS_nowUsing = pin_CS;
	IO_CS1_End;
	GPIOx_BLK_nowUsing = type_BLK;
	GPIO_Pin_BLK_nowUsing = pin_BLK;
	IO_BLK1_Open;

	GPIOx_RST_nowUsing = type_RST;
	GPIO_Pin_RST_nowUsing = pin_RST;
	IO_RST0_StartRST;
	HAL_Delay(120);
	IO_RST1_EndRST;
	////////////////////////////////////////
	SendCommand(0x11); // 无此指令，不能正常初始化芯片，无显示
	HAL_Delay(120);
	SendCommand(0x36); // 设置内存扫描方向，0X00正常扫描，从上往下，从左往右，RGB方式

	if (USE_HORIZONTAL == 0)
		SendDATA8(0x00);
	else if (USE_HORIZONTAL == 1)
		SendDATA8(0xC0);
	else if (USE_HORIZONTAL == 2)
		SendDATA8(0x70);
	else
		SendDATA8(0xA0);

	SendCommand(0x3A); // 数据格式，65K色,565
	SendDATA8(0x05);

	SendCommand(0xB2); // 帧频设置
	SendDATA8(0x0C);
	SendDATA8(0x0C);
	SendDATA8(0x00);
	SendDATA8(0x33);
	SendDATA8(0x33);

	SendCommand(0xB7); // GATE 设置
	SendDATA8(0x35);

	SendCommand(0xBB); // VCOM设置
	SendDATA8(0x19);

	SendCommand(0xC0); // LCM设置,默认0x2c
	SendDATA8(0x2C);

	SendCommand(0xC2); // VDV&VRH SET ,默认0x01
	SendDATA8(0x01);

	SendCommand(0xC3); // VRHS SET，默认0x0b
	SendDATA8(0x12);   // 此处根据实际情况修正

	SendCommand(0xC4); // VDV SET，默认0x20
	SendDATA8(0x20);

	SendCommand(0xC6); // FR SET, 默认0x0F
	SendDATA8(0x0F);

	SendCommand(0xD0); // 电源控制1
	SendDATA8(0xA4);   // 该参数不变
	SendDATA8(0xA1);   // 此处根据实际情况修改

	SendCommand(0xE0); // 正极性GAMMA调整
	SendDATA8(0xD0);
	SendDATA8(0x04);
	SendDATA8(0x0D);
	SendDATA8(0x11);
	SendDATA8(0x13);
	SendDATA8(0x2B);
	SendDATA8(0x3F);
	SendDATA8(0x54);
	SendDATA8(0x4C);
	SendDATA8(0x18);
	SendDATA8(0x0D);
	SendDATA8(0x0B);
	SendDATA8(0x1F);
	SendDATA8(0x23);

	SendCommand(0xE1); // 负极性GAMMA调整
	SendDATA8(0xD0);
	SendDATA8(0x04);
	SendDATA8(0x0C);
	SendDATA8(0x11);
	SendDATA8(0x13);
	SendDATA8(0x2C);
	SendDATA8(0x3F);
	SendDATA8(0x44);
	SendDATA8(0x51);
	SendDATA8(0x2F);
	SendDATA8(0x1F);
	SendDATA8(0x1F);
	SendDATA8(0x20);
	SendDATA8(0x23);

	// SendCommand(0x21); // 反显开，默认是0X20，正常模式

	SendCommand(0x29);
}
#endif

void MyTFT_OpenScreen()
{
	IO_BLK1_Open;
}
void MyTFT_CloseScreen()
{
	IO_BLK0_CLose;
}