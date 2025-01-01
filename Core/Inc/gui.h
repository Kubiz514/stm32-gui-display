#pragma once

void draw_top_bar(uint8_t menu_bar_height);
void draw_joystick_demo(uint8_t top_offset, uint8_t TOP_BAR_HEIGHT, uint16_t adc_reading_x,
		uint16_t adc_reading_y);
void draw_distance_sensor_demo(uint8_t top_offset, uint8_t top_bar_height, uint16_t distance_reading);
void draw_text_and_shapes_demo(uint8_t top_bar_height);



