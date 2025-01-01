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
#include <wchar.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "lcd.h"
#include "gui.h"
#include "profiling.h"

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

bool IS_RENDER_PROFILING_ENABLED = true;
bool IS_LCD_TRANSFER_PROFILING_ENABLED = true;

static uint32_t GUI_PAGES_COUNT = 3;
typedef enum {
  JOYSTICK_DEMO,
  DISTANCE_SENSOR_DEMO,
  STATIC_SCREEN_DEMO
}gui_page;
// starting index can be changed here
volatile uint32_t gui_screen_index = 0;

volatile bool are_buttons_debounced = true;

// interrupts

void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
	if (hspi == &hspi2)
	{
		lcd_data_transmit_done();

		finish_lcd_data_transfer_profiling();
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim == &htim1)
	{
		if(HAL_GPIO_ReadPin(BTN1_IN_GPIO_Port, BTN1_IN_Pin) == GPIO_PIN_RESET ||
		   HAL_GPIO_ReadPin(BTN2_IN_GPIO_Port, BTN2_IN_Pin) == GPIO_PIN_RESET)
		{
			are_buttons_debounced = true;
			if(HAL_TIM_Base_Stop_IT(&htim1) != HAL_OK)
				Error_Handler();
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

			if(HAL_TIM_Base_Start_IT(&htim1) != HAL_OK)
				Error_Handler();
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

			if(HAL_TIM_Base_Start_IT(&htim1) != HAL_OK)
				Error_Handler();
			are_buttons_debounced = false;
		}
	}
}

// program logic

int __io_putchar(int character)
{
	if (character == '\n')
	{
		uint8_t inserted_carriage_return = '\r';
		if(HAL_UART_Transmit(&huart2, &inserted_carriage_return, 1, HAL_MAX_DELAY) != HAL_OK)
			Error_Handler();
	}

	if(HAL_UART_Transmit(&huart2, (uint8_t*)&character, 1, HAL_MAX_DELAY) != HAL_OK)
		Error_Handler();
	return 1;
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
	if(HAL_TIM_IC_Start(&htim3, TIM_CHANNEL_1) != HAL_OK)
		Error_Handler();
	if(HAL_TIM_IC_Start(&htim3, TIM_CHANNEL_2) != HAL_OK)
		Error_Handler();
	if(HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3) != HAL_OK)
		Error_Handler();

	// initialize lcd
	HAL_Delay(1000);
	lcd_init();

	// setup GUI
	uint8_t TOP_BAR_HEIGHT = 20;

	// setup joysticks ADC
	volatile static uint16_t adc1_readings[2];

	// setup ADC1 reading with DMA
	// channel 1: joystick X
	// channel 2: joystick Y

	if(HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
		Error_Handler();
	if(HAL_ADC_Start_DMA(&hadc1, (uint32_t*) adc1_readings, 2) != HAL_OK)
		Error_Handler();

	init_profiling_timers();


	while (1)
	{
		// read distance sensor
		uint32_t distance_sensor_start = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_1);
		uint32_t distance_sensor_stop = HAL_TIM_ReadCapturedValue(&htim3, TIM_CHANNEL_2);

		// update screen
		if (!is_lcd_data_being_transmitted())
		{
			start_render_profiling();

			/* CONSISTENT GUI ELEMENTS BEGIN */
			draw_top_bar(TOP_BAR_HEIGHT);
			/* CONSISTENT GUI ELEMENTS END */

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

			finish_render_profiling();
			lcd_copy_data();
			start_lcd_data_transfer_profiling();
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
