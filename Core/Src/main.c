/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
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
#include "adc.h"
#include "dma.h"
#include "spi.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "hagl.h"
#include "font6x9.h"
#include "rgb565.h"
#include <wchar.h>
#include <string.h>
#include "math_utils.h"

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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static uint32_t GUI_PAGES_COUNT = 3;
volatile uint32_t gui_screen_index = 0;

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi == &hspi2)
	{
		lcd_transfer_done();
	}
}

//Button interrupt
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	//Next page button
	if (GPIO_Pin == BTN1_IN_Pin) {
		//circular loop
		if(gui_screen_index >= GUI_PAGES_COUNT - 1)
			gui_screen_index = 0;
		else
			gui_screen_index++;
	}

	//Previous page button
	if (GPIO_Pin == BTN2_IN_Pin) {
		//circular loop
		if(gui_screen_index == 0)
			gui_screen_index = GUI_PAGES_COUNT - 1;
		else
			gui_screen_index--;
	}
}

void draw_text(uint32_t number_input, int16_t x0, int16_t y0)
{
	wchar_t text_buffer[5];
	swprintf(text_buffer, 5, L"%d", number_input);
	hagl_put_text(text_buffer, x0, y0, YELLOW, font6x9);
}


void draw_number_as_text(uint32_t number_input, int16_t x0, int16_t y0)
{
	wchar_t text_buffer[5];
	swprintf(text_buffer, 5, L"%d", number_input);
	hagl_put_text(text_buffer, x0, y0, YELLOW, font6x9);
}

int32_t map_adc_values(int32_t input_value, int32_t min_output,
		int32_t max_output)
{
	// ADC values range from 0 to 4095
	return map_ranges(input_value, 0, 4095, min_output, max_output);
}

void draw_menu_top_bar(uint8_t menu_bar_height)
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

void draw_joystick_demo2(uint8_t top_offset, uint8_t top_bar_height, uint16_t adc_reading_x,
		uint16_t adc_reading_y)
{

	uint8_t dot_x = map_adc_values(adc_reading_x, 0, LCD_WIDTH);
	uint8_t dot_y = map_adc_values(adc_reading_y, top_bar_height + 1, LCD_HEIGHT);

	// draw "dot area" background
	hagl_fill_rectangle(0, top_bar_height, LCD_WIDTH, LCD_HEIGHT, YELLOW);

	// draw red dot based on ADC1 joystick input
	hagl_fill_circle(dot_x, dot_y, 5, BLUE);

	// draw section above "dot area" displaying ADC values as text
	hagl_fill_rectangle(0, top_offset, LCD_WIDTH, top_bar_height, GREEN);
	draw_number_as_text(adc_reading_x, 80, 25);
	draw_number_as_text(adc_reading_y, 120, 25);
}

void draw_joystick_demo3(uint8_t top_offset, uint8_t top_bar_height, uint16_t adc_reading_x,
		uint16_t adc_reading_y)
{

	uint8_t dot_x = map_adc_values(adc_reading_x, 0, LCD_WIDTH);
	uint8_t dot_y = map_adc_values(adc_reading_y, top_bar_height + 1, LCD_HEIGHT);

	// draw "dot area" background
	hagl_fill_rectangle(0, top_bar_height, LCD_WIDTH, LCD_HEIGHT, YELLOW);

	// draw red dot based on ADC1 joystick input
	hagl_fill_circle(dot_x, dot_y, 5, GREEN);

	// draw section above "dot area" displaying ADC values as text
	hagl_fill_rectangle(0, top_offset, LCD_WIDTH, top_bar_height, GREEN);
	draw_number_as_text(adc_reading_x, 80, 25);
	draw_number_as_text(adc_reading_y, 120, 25);
}

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
  MX_SPI2_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	lcd_init();

	// setup joystick demo
	volatile static uint16_t adc1_readings[2];

	// setup ADC1 reading with DMA
	// channel 1: joystick X
	// channel 2: joystick Y

	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc1_readings, 2);

	while (1)
	{
		// update screen
		if (!lcd_is_busy())
		{
			draw_menu_top_bar(20);

			/* GUI PAGES BEGIN */
			if(gui_screen_index == 0)
				draw_joystick_demo(20, 40, adc1_readings[0], adc1_readings[1]);
			else if(gui_screen_index == 1)
				draw_joystick_demo2(20, 40, adc1_readings[0], adc1_readings[1]);
			else if(gui_screen_index == 2)
				draw_joystick_demo3(20, 40, adc1_readings[0], adc1_readings[1]);
			/* GUI PAGES END */

			lcd_copy();
		}

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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

#ifdef  USE_FULL_ASSERT
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
