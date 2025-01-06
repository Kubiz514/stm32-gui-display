#include "font6x9.h"
#include "rgb565.h"
#include "gui.h"
#include "hagl.h"
#include "math_utils.h"
#include <stdint.h>
#include <stdbool.h>

void draw_number_as_text(uint32_t number_input, int16_t x0, int16_t y0)
{
	// max uint32_t is 10 digits
	wchar_t text_buffer[11];
	swprintf(text_buffer, 11, L"%d", number_input);
	hagl_put_text(text_buffer, x0, y0, YELLOW, font6x9);
}

int32_t map_adc_values(int32_t input_value, int32_t min_output,
		int32_t max_output)
{
	// ADC values range from 0 to 4095
	return map_ranges(input_value, 0, 4095, min_output, max_output);
}

void draw_top_bar(uint8_t menu_bar_height)
{
	// draw menu background
	hagl_fill_rectangle(0, 0, LCD_WIDTH, menu_bar_height, BLACK);

	// draw left rectangle
	hagl_fill_triangle(10, 10, 30, 5, 30, 15, RED);

	// draw right rectangle
	hagl_fill_triangle(130, 5, 130, 15, 150, 10, RED);
}

void draw_joystick_demo(uint8_t top_offset, uint8_t top_bar_height, uint16_t adc_reading_x,
		uint16_t adc_reading_y)
{

	uint8_t dot_x = map_adc_values(adc_reading_x, 0, LCD_WIDTH);
	uint8_t dot_y = map_adc_values(adc_reading_y, top_bar_height + 1, LCD_HEIGHT);

	// draw "dot area" background
	hagl_fill_rectangle(0, top_bar_height, LCD_WIDTH, LCD_HEIGHT, YELLOW);

	// draw red dot based on ADC1 joystick input
	hagl_fill_circle(dot_x, dot_y, 5, RED);

	// draw section above "dot area" displaying ADC values as text
	hagl_fill_rectangle(0, top_offset, LCD_WIDTH, top_bar_height, GREEN);
	draw_number_as_text(adc_reading_x, 80, 25);
	draw_number_as_text(adc_reading_y, 120, 25);
}

void draw_distance_sensor_demo(uint8_t top_offset, uint8_t top_bar_height, uint16_t distance_reading)
{
	const uint16_t MAX_VALID_DISTANCE = 94;
	const uint16_t DISTANCE_BAR_BORDER = 15;

	// draw background
	hagl_fill_rectangle(0, 20, LCD_WIDTH, LCD_HEIGHT, BLUE);

	hagl_fill_rectangle(5, 59, LCD_WIDTH - 5, 89, WHITE);
	hagl_fill_rectangle(DISTANCE_BAR_BORDER, 69, LCD_WIDTH - DISTANCE_BAR_BORDER, 79, BLACK);

	// cap incorrect reading at max
	if(distance_reading > MAX_VALID_DISTANCE)
	{
		distance_reading = MAX_VALID_DISTANCE;
	}

	int16_t distance_mapped_onto_x = (int16_t)map_ranges((int32_t)distance_reading, 0, MAX_VALID_DISTANCE, DISTANCE_BAR_BORDER, LCD_WIDTH - 30);
	hagl_fill_rectangle(DISTANCE_BAR_BORDER, 69, distance_mapped_onto_x, 79, RED);
}

void draw_text_and_shapes_demo(uint8_t top_bar_height)
{
	hagl_fill_rectangle(0, top_bar_height, LCD_WIDTH, LCD_HEIGHT, BLACK);
	hagl_fill_circle(30, 50, 10, GREEN);
	hagl_fill_rectangle(70, 40, 90, 60, RED);
	int16_t vertices[6] = {120, 60, 130, 40, 140, 60};
	hagl_fill_polygon(3, vertices, CYAN);

	hagl_put_text(L"The quick brown fox", 20, 80, YELLOW, font6x9);
	hagl_put_text(L"jumps over the lazy dog", 20, 100, YELLOW, font6x9);
}

