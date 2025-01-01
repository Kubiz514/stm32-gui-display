#include "tim.h"
#include <stdio.h>
#include <stdbool.h>

void init_profiling_timers(void)
{
	if(IS_LCD_TRANSFER_PROFILING_ENABLED)
	{
		if(HAL_TIM_Base_Start_IT(&htim16) != HAL_OK)
			Error_Handler();
	}

	if(IS_RENDER_PROFILING_ENABLED)
	{
		if(HAL_TIM_Base_Start_IT(&htim17) != HAL_OK)
			Error_Handler();
	}
}

void start_render_profiling(void)
{
	if(IS_RENDER_PROFILING_ENABLED)
	{
		__HAL_TIM_SET_COUNTER(&htim17, 0);
	}
}

void finish_render_profiling(void)
{
	if(IS_RENDER_PROFILING_ENABLED)
	{
		uint16_t render_time = __HAL_TIM_GET_COUNTER(&htim17);
		printf("render: %.1f ms\r\n", (float)render_time/10);
	}
}

void start_lcd_data_transfer_profiling(void)
{
	if(IS_LCD_TRANSFER_PROFILING_ENABLED)
	{
		__HAL_TIM_SET_COUNTER(&htim16, 0);
	}
}

void finish_lcd_data_transfer_profiling(void)
{
	if(IS_LCD_TRANSFER_PROFILING_ENABLED)
	{
		uint16_t lcd_transfer_time = __HAL_TIM_GET_COUNTER(&htim16);
		printf("lcd data transfer: %.1f ms\r\n", (float)lcd_transfer_time/10);
	}
}
