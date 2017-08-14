/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"
#include "cmsis_os.h"
#include "crc.h"
#include "fatfs.h"
#include "sdio.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "wwdg.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);
void MX_FREERTOS_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
osThreadId debugTaskHandle;

FATFS SDDISKFatFs;  /* File system object for RAM disk logical drive */
FIL MyFile;          /* File object */
extern char SD_Path[];/* StartDefaultTask function */
FRESULT res;                                          /* FatFs function common result code */
uint32_t byteswritten, bytesread;                     /* File write/read counts */
uint8_t wtext[] = "This is STM32 working with FatFs es pina"; /* File write buffer */
uint8_t rtext[100];

void debugTask(void const * argument) {

	osDelay(100);

	/*##-2- Register the file system object to the FatFs module ##############*/
	if(f_mount(&SDDISKFatFs, (TCHAR const*)SD_Path, 0) != FR_OK)
	{
	  /* FatFs Initialization Error */
	  Error_Handler();
	}
	else
	{
	  /*##-3- Create a FAT file system (format) on the logical drive #########*/
	  if(f_mkfs((TCHAR const*)SD_Path, 0, 0) != FR_OK)
	  {
		/* FatFs Format Error */
		Error_Handler();
	  }
	  else
	  {
		/*##-4- Create and Open a new text file object with write access #####*/
		if(f_open(&MyFile, "STM32.TXT", FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
		{
		  /* 'STM32.TXT' file Open for write Error */
		  Error_Handler();
		}
		else
		{
		  /*##-5- Write data to the text file ################################*/
		  res = f_write(&MyFile, wtext, sizeof(wtext), (void *)&byteswritten);

		  if((byteswritten == 0) || (res != FR_OK))
		  {
			/* 'STM32.TXT' file Write or EOF Error */
			Error_Handler();
		  }
		  else
		  {
			/*##-6- Close the open text file #################################*/
			f_close(&MyFile);

			/*##-7- Open the text file object with read access ###############*/
			if(f_open(&MyFile, "STM32.TXT", FA_READ) != FR_OK)
			{
			  /* 'STM32.TXT' file Open for read Error */
			  Error_Handler();
			}
			else
			{
			  /*##-8- Read data from the text file ###########################*/
			  res = f_read(&MyFile, rtext, sizeof(rtext), (void *)&bytesread);

			  if((bytesread == 0) || (res != FR_OK))
			  {
				/* 'STM32.TXT' file Read or EOF Error */
				Error_Handler();
			  }
			  else
			  {
				/*##-9- Close the open text file #############################*/
				f_close(&MyFile);

				/*##-10- Compare read data with the expected data ############*/
				if((bytesread != byteswritten))
				{
				  /* Read data is different from the expected data */
				  Error_Handler();
				}
				else
				{
				  /* Success of the demo: no error occurrence */
				  ;
				}
			  }
			}
		  }
		}
	  }
	}

	while(1){
		osDelay(1);
	}
}

osThreadId buttonReadTaskHandle;

GPIO_PinState button1_state, button2_state;
void buttonReadTask(void const * argument) {




	while(1) {
		if( HAL_GPIO_ReadPin(GPIOB, button_1_Pin) == GPIO_PIN_SET ) {
			button1_state = GPIO_PIN_SET;
		} else {
			button1_state = GPIO_PIN_RESET;
		}



		if( HAL_GPIO_ReadPin(GPIOB, button_2_Pin) == GPIO_PIN_SET &&
				button2_state == GPIO_PIN_RESET) {
			button2_state = GPIO_PIN_SET;
		};
		if( HAL_GPIO_ReadPin(GPIOB, button_2_Pin) == GPIO_PIN_RESET &&
				button2_state == GPIO_PIN_SET) {
			button2_state = GPIO_PIN_RESET;
		};
		osDelay(100);

	}

}
uint8_t uartReadBuffer[100];
uint8_t puartReadBuffer=0;
uint8_t lineEndReceived=0;
void processMsg( void ) {
	int i;
	for(i=0;i<puartReadBuffer;i++){
		puartReadBuffer=0;
		uartReadBuffer[i]=0;
	}
}
void notifyReadTask(char data_in) {
	uartReadBuffer[puartReadBuffer] = data_in;
	puartReadBuffer++;
	if(data_in == '\n' || data_in == '\r') {
		lineEndReceived = 1;
	}
}
osThreadId uartReadTaskHandle;

void uartReadTask(void const * argument) {

	HAL_UART_Transmit_IT(USART2_getHandle(), "Nucleo alive!\r\n", 14);
	while(1) {
		if(lineEndReceived) {
			processMsg();
		}
		osDelay(1000);
	}

}
/* USER CODE END 0 */
extern uint8_t uartReadByte;
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
//  MX_TIM10_Init();
  MX_USART2_UART_Init();
  HAL_UART_Receive_IT(USART2_getHandle(), &uartReadByte, 1);
  //MX_WWDG_Init();
//  MX_SPI1_Init();
  MX_CRC_Init();
//  MX_TIM7_Init();
  MX_SDIO_SD_Init();

  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  //osThreadDef(debug, debugTask, osPriorityNormal, 0, 128);
  //debugTaskHandle = osThreadCreate(osThread(debug), NULL);

  osThreadDef(button, buttonReadTask, osPriorityNormal, 0, 128);
  buttonReadTaskHandle = osThreadCreate(osThread(button), NULL);

  osThreadDef(uart, uartReadTask, osPriorityNormal, 0, 128);
  uartReadTaskHandle = osThreadCreate(osThread(uart), NULL);
  /* Start scheduler */
  osKernelStart();
  
  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */

}


/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 3;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Activate the Over-Drive mode 
    */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SDIO|RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.PLLSAI.PLLSAIM = 8;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
  PeriphClkInitStruct.PLLSAI.PLLSAIQ = 2;
  PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV4;
  PeriphClkInitStruct.PLLSAIDivQ = 1;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48CLKSOURCE_PLLSAIP;
  PeriphClkInitStruct.SdioClockSelection = RCC_SDIOCLKSOURCE_CLK48;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM4 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
/* USER CODE BEGIN Callback 0 */

/* USER CODE END Callback 0 */
  if (htim->Instance == TIM4) {
    HAL_IncTick();
  }
/* USER CODE BEGIN Callback 1 */

/* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
