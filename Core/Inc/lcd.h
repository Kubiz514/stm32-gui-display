#pragma once

#include <stdint.h>
#include <stdbool.h>

#define LCD_WIDTH	160
#define LCD_HEIGHT	128

// Little endian - RGB is reversed
#define BLACK			0x0000
#define RED			0x00f8
#define GREEN			0xe007
#define BLUE			0x1f00
#define YELLOW			0xe0ff
#define MAGENTA			0x1ff8
#define CYAN			0xff07
#define WHITE			0xffff

void lcd_init(void);
void set_pixel_in_buffer(int x, int y, uint16_t color);
void lcd_transmit_data(void);
void lcd_data_transmit_done(void);
bool is_lcd_data_being_transmitted(void);
