#include "lcd.h"
#include "spi.h"

// LCD commands
#define ST7735S_SLPOUT			0x11
#define ST7735S_DISPOFF			0x28
#define ST7735S_DISPON			0x29
#define ST7735S_CASET			0x2a
#define ST7735S_RASET			0x2b
#define ST7735S_RAMWR			0x2c
#define ST7735S_MADCTL			0x36
#define ST7735S_COLMOD			0x3a
#define ST7735S_FRMCTR1			0xb1
#define ST7735S_FRMCTR2			0xb2
#define ST7735S_FRMCTR3			0xb3
#define ST7735S_INVCTR			0xb4
#define ST7735S_PWCTR1			0xc0
#define ST7735S_PWCTR2			0xc1
#define ST7735S_PWCTR3			0xc2
#define ST7735S_PWCTR4			0xc3
#define ST7735S_PWCTR5			0xc4
#define ST7735S_VMCTR1			0xc5
#define ST7735S_GAMCTRP1		0xe0
#define ST7735S_GAMCTRN1		0xe1

// Other defines
#define LCD_OFFSET_X  1
#define LCD_OFFSET_Y  2
#define CMD(x)			((x) | 0x100)

static uint16_t pixels_buffer[LCD_WIDTH * LCD_HEIGHT];

// commands require different pin configuration than regular data being sent

static void lcd_transmit_cmd(uint8_t cmd)
{
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	if(HAL_SPI_Transmit(&hspi2, &cmd, 1, HAL_MAX_DELAY) != HAL_OK)
		Error_Handler();
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

static void lcd_transmit_data(uint8_t data)
{
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	if(HAL_SPI_Transmit(&hspi2, &data, 1, HAL_MAX_DELAY) != HAL_OK)
		Error_Handler();
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

static void lcd_send(uint16_t value)
{
	if (value & 0x100)
	{
		lcd_transmit_cmd(value);
	}
	else
	{
		lcd_transmit_data(value);
	}
}

static const uint16_t lcd_init_table[] = {
	CMD(ST7735S_FRMCTR1), 0x01, 0x2c, 0x2d,
	CMD(ST7735S_FRMCTR2), 0x01, 0x2c, 0x2d,
	CMD(ST7735S_FRMCTR3), 0x01, 0x2c, 0x2d, 0x01, 0x2c, 0x2d,
	CMD(ST7735S_INVCTR), 0x07,
	CMD(ST7735S_PWCTR1), 0xa2, 0x02, 0x84,
	CMD(ST7735S_PWCTR2), 0xc5,
	CMD(ST7735S_PWCTR3), 0x0a, 0x00,
	CMD(ST7735S_PWCTR4), 0x8a, 0x2a,
	CMD(ST7735S_PWCTR5), 0x8a, 0xee,
	CMD(ST7735S_VMCTR1), 0x0e,
	CMD(ST7735S_GAMCTRP1), 0x0f, 0x1a, 0x0f, 0x18, 0x2f, 0x28, 0x20, 0x22,
						 0x1f, 0x1b, 0x23, 0x37, 0x00, 0x07, 0x02, 0x10,
	CMD(ST7735S_GAMCTRN1), 0x0f, 0x1b, 0x0f, 0x17, 0x33, 0x2c, 0x29, 0x2e,
						 0x30, 0x30, 0x39, 0x3f, 0x00, 0x07, 0x03, 0x10,
	CMD(0xf0), 0x01,
	CMD(0xf6), 0x00,
	CMD(ST7735S_COLMOD), 0x05,
	CMD(ST7735S_MADCTL), 0xa0,
};

void lcd_init(void)
{
	int init_table_index;

	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
	HAL_Delay(100);
	HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
	HAL_Delay(100);

	for (init_table_index = 0; init_table_index < sizeof(lcd_init_table) / sizeof(uint16_t); init_table_index++)
	{
		lcd_send(lcd_init_table[init_table_index]);
	}

	HAL_Delay(200);

	lcd_transmit_cmd(ST7735S_SLPOUT);
	HAL_Delay(120);

	lcd_transmit_cmd(ST7735S_DISPON);
}

static void lcd_transmit_data16(uint16_t value)
{
	lcd_transmit_data(value >> 8);
	lcd_transmit_data(value);
}

static void lcd_set_window(int x, int y, int width, int height)
{
	lcd_transmit_cmd(ST7735S_CASET);
	lcd_transmit_data16(LCD_OFFSET_X + x);
	lcd_transmit_data16(LCD_OFFSET_X + x + width - 1);

	lcd_transmit_cmd(ST7735S_RASET);
	lcd_transmit_data16(LCD_OFFSET_Y + y);
	lcd_transmit_data16(LCD_OFFSET_Y + y + height- 1);
}


void set_pixel_in_buffer(int x, int y, uint16_t color)
{
	pixels_buffer[x + y * LCD_WIDTH] = color;
}

void lcd_copy_data(void)
{
	lcd_set_window(0, 0, LCD_WIDTH, LCD_HEIGHT);
	lcd_transmit_cmd(ST7735S_RAMWR);

	// pins required for SPI data transmission
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	if(HAL_SPI_Transmit_DMA(&hspi2, (uint8_t*)pixels_buffer, sizeof(pixels_buffer)) != HAL_OK)
		Error_Handler();
}

void lcd_data_transmit_done(void)
{
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

bool is_lcd_data_being_transmitted(void)
{
	if (HAL_GPIO_ReadPin(LCD_CS_GPIO_Port, LCD_CS_Pin) == GPIO_PIN_RESET)
	{
		return true;
	}
	else
	{
		return false;
	}
}
