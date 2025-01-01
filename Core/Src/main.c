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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "hagl.h"
#include "font6x9.h"
#include "rgb565.h"
#include <wchar.h>
#include <string.h>
#include <stdio.h>
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
typedef enum {
  JOYSTICK_DEMO,
  DISTANCE_SENSOR_DEMO,
  STATIC_SCREEN_DEMO
}gui_page;


volatile bool are_buttons_debounced = true;
static bool IS_RENDER_PROFILING_ENABLED = true;
static bool IS_LCD_TRANSFER_PROFILING_ENABLED = true;

// interrupts

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi == &hspi2)
	{
		lcd_data_transmit_done();

		if(IS_LCD_TRANSFER_PROFILING_ENABLED)
		{
			uint16_t lcd_transfer_time = __HAL_TIM_GET_COUNTER(&htim16);
			printf("lcd data transfer: %.1f ms\r\n", (float)lcd_transfer_time/10);
		}
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim == &htim1)
	{
		if(HAL_GPIO_ReadPin(BTN1_IN_GPIO_Port, BTN1_IN_Pin) == GPIO_PIN_RESET ||
		   HAL_GPIO_ReadPin(BTN2_IN_GPIO_Port, BTN2_IN_Pin) == GPIO_PIN_RESET){
			are_buttons_debounced = true;
			HAL_TIM_Base_Stop_IT(&htim1);
		}
	}

	  if (htim == &htim16)
	  {
		  printf("LCD data transfer timer MAX at 6 s\r\n");
	  }

	  if (htim == &htim17)
	  {
		  printf("Render timer MAX at 6 s\r\n");
	  }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(are_buttons_debounced)
	{
		//Next page button
		if (GPIO_Pin == BTN1_IN_Pin)
		{
			//circular loop
			if(gui_screen_index >= GUI_PAGES_COUNT - 1)
				gui_screen_index = 0;
			else
				gui_screen_index++;

			HAL_TIM_Base_Start_IT(&htim1);
			are_buttons_debounced = false;
		}

		//Previous page button
		if (GPIO_Pin == BTN2_IN_Pin)
		{
			//circular loop
			if(gui_screen_index <= 0)
				gui_screen_index = GUI_PAGES_COUNT - 1;
			else
				gui_screen_index--;

			HAL_TIM_Base_Start_IT(&htim1);
			are_buttons_debounced = false;
		}
	}
}

// program logic

int __io_putchar(int ch)
{
    if (ch == '\n') {
        uint8_t ch2 = '\r';
        HAL_UART_Transmit(&huart2, &ch2, 1, HAL_MAX_DELAY);
    }

    HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return 1;
}

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

// GUI elements

void draw_top_bar(uint8_t menu_bar_height)
{
	// draw menu background
	hagl_fill_rectangle(0, 0, LCD_WIDTH, menu_bar_height, BLACK);

	// draw left rectangle
	hagl_fill_triangle(10, 10, 30, 5, 30, 15, RED);

	// draw right rectangle
	hagl_fill_triangle(130, 5, 130, 15, 150, 10, RED);
}

void draw_joystick_demo(uint8_t top_offset, uint8_t TOP_BAR_HEIGHT, uint16_t adc_reading_x,
		uint16_t adc_reading_y)
{

	uint8_t dot_x = map_adc_values(adc_reading_x, 0, LCD_WIDTH);
	uint8_t dot_y = map_adc_values(adc_reading_y, TOP_BAR_HEIGHT + 1, LCD_HEIGHT);

	// draw "dot area" background
	hagl_fill_rectangle(0, TOP_BAR_HEIGHT, LCD_WIDTH, LCD_HEIGHT, YELLOW);

	// draw red dot based on ADC1 joystick input
	hagl_fill_circle(dot_x, dot_y, 5, RED);

	// draw section above "dot area" displaying ADC values as text
	hagl_fill_rectangle(0, top_offset, LCD_WIDTH, TOP_BAR_HEIGHT, GREEN);
	draw_number_as_text(adc_reading_x, 80, 25);
	draw_number_as_text(adc_reading_y, 120, 25);
}

void draw_distance_sensor_demo(uint8_t top_offset, uint8_t TOP_BAR_HEIGHT, uint16_t distance_reading)
{
	const uint16_t MAX_VALID_DISTANCE = 94;
	const uint16_t DISTANCE_BAR_BORDER = 15;

	// draw background
	hagl_fill_rectangle(0, 20, LCD_WIDTH, LCD_HEIGHT, BLUE);

	// todo add method which draws rectangle border, or just clean up magic numbers
	// draw distance bar
	hagl_fill_rectangle(5, 59, LCD_WIDTH - 5, 89, WHITE);
	hagl_fill_rectangle(DISTANCE_BAR_BORDER, 69, LCD_WIDTH - DISTANCE_BAR_BORDER, 79, BLACK);

	// cap incorrect reading at max
	if(distance_reading > MAX_VALID_DISTANCE)
	{
		distance_reading = MAX_VALID_DISTANCE;
	}

	// todo make sure int32 is mapped onto int 16 correctly
	int16_t distance_mapped_onto_x = (int16_t)map_ranges((int32_t)distance_reading, 0, MAX_VALID_DISTANCE, DISTANCE_BAR_BORDER, LCD_WIDTH - 30);
	hagl_fill_rectangle(DISTANCE_BAR_BORDER, 69, distance_mapped_onto_x, 79, RED);
}

void draw_text_and_shapes_demo(uint8_t TOP_BAR_HEIGHT)
{
	hagl_fill_rectangle(0, TOP_BAR_HEIGHT, LCD_WIDTH, LCD_HEIGHT, BLACK);
	hagl_fill_circle(30, 50, 10, GREEN);
	hagl_fill_rectangle(70, 40, 90, 60, RED);
	int16_t vertices[6] = {120, 60, 130, 40, 140, 60};
	hagl_fill_polygon(3, vertices, CYAN);

	hagl_put_text(L"The quick brown fox", 20, 80, YELLOW, font6x9);
	hagl_put_text(L"jumps over the lazy dog", 20, 100, YELLOW, font6x9);
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
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  MX_TIM1_Init();
  MX_TIM17_Init();
  MX_TIM16_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  // init distance sensor timer channels
  HAL_TIM_IC_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_IC_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);

  // initialize lcd
  HAL_Delay(1000);
  lcd_init();

  // setup GUI
  static uint8_t TOP_BAR_HEIGHT = 20;

	// setup joystick demo
	volatile static uint16_t adc1_readings[2];

	// setup ADC1 reading with DMA
	// channel 1: joystick X
	// channel 2: joystick Y

	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc1_readings, 2);

	// start profiling timers
	if(IS_LCD_TRANSFER_PROFILING_ENABLED)
	{
		HAL_TIM_Base_Start_IT(&htim16);
	}

	if(IS_RENDER_PROFILING_ENABLED)
	{
		HAL_TIM_Base_Start_IT(&htim17);
	}

	while (1)
	{
		// read distance sensor
		uint32_t distance_sensor_start = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_1);
		uint32_t distance_sensor_stop = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_2);

		// update screen
		if (!is_lcd_data_being_transmitted())
		{
			if(IS_RENDER_PROFILING_ENABLED)
			{
				__HAL_TIM_SET_COUNTER(&htim17, 0);
			}

			draw_top_bar(TOP_BAR_HEIGHT);

			/* GUI PAGES BEGIN */
			if(gui_screen_index == JOYSTICK_DEMO)
			{
				draw_joystick_demo(TOP_BAR_HEIGHT, 40, adc1_readings[0], adc1_readings[1]);
			}
			else if(gui_screen_index == DISTANCE_SENSOR_DEMO)
			{
				draw_distance_sensor_demo(TOP_BAR_HEIGHT, 40, (uint16_t)(distance_sensor_stop - distance_sensor_start));
			}
			else if(gui_screen_index == STATIC_SCREEN_DEMO)
			{
				draw_text_and_shapes_demo(TOP_BAR_HEIGHT);
			}
			/* GUI PAGES END */

			if(IS_RENDER_PROFILING_ENABLED)
			{
				uint16_t render_time = __HAL_TIM_GET_COUNTER(&htim17);
				printf("render: %.1f ms\r\n", (float)render_time/10);
			}

			lcd_transmit_data();

			if(IS_LCD_TRANSFER_PROFILING_ENABLED)
			{
				__HAL_TIM_SET_COUNTER(&htim16, 0);
			}
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
