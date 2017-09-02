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
#include "display.h"

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
uint8_t wtext[] = "This is STM32 working with FatFs"; /* File write buffer */
uint8_t rtext[100];


TaskStatus_t task_3;
void debugTask(void const * argument) {

	osDelay(100);
	/*##-2- Register the file system object to the FatFs module ##############*/
	if(f_mount(&SDDISKFatFs, "", 1/*(TCHAR const*)SD_Path, 0*/) != FR_OK)
	{
		HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"SD mount not OK\r", 16);
		osDelay(100);
		/* FatFs Initialization Error */
		Error_Handler();
	}
	else
	{
		HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"SD mount OK\r", 12);
        	osDelay(100);
//		if(f_mkfs((TCHAR const*)SD_Path, 0, 0) != FR_OK)
//		{
//			HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"Make fs not OK\r", 15);
//			osDelay(100);
//			/* FatFs Format Error */
//			Error_Handler();
//		}
//		else
		{
        	 HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"Make fs OK\r", 11);
             osDelay(100);

			/*##-4- Create and Open a new text file object with write access #####*/
			if(f_open(&MyFile, "STM32asd.TXT", FA_CREATE_ALWAYS | FA_WRITE) != FR_OK)
			{
				 HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"fopen not OK\r", 13);
                 osDelay(100);

				/* 'STM32.TXT' file Open for write Error */
				Error_Handler();
			}
			else
			{
				 HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"fopen OK\r", 9);
                 osDelay(100);

                 /*##-5- Write data to the text file ################################*/
				 res = f_write(&MyFile, wtext, sizeof(wtext), (void *)&byteswritten);

				 if((byteswritten == 0) || (res != FR_OK))
				{
					 HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"fwrite not OK\r", 14);
                     osDelay(100);

					/* 'STM32.TXT' file Write or EOF Error */
					Error_Handler();
				}
				else
				{
					HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"fwrite OK\r", 10);
                    osDelay(100);
					/*##-6- Close the open text file #################################*/
					f_close(&MyFile);
                    HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"File closed, read back!\r", 24);
                    osDelay(100);


					/*##-7- Open the text file object with read access ###############*/
					if(f_open(&MyFile, "STM32asd.TXT", FA_READ) != FR_OK)
					{
						HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"fopen not OK\r", 13);
				        osDelay(100);

						/* 'STM32.TXT' file Open for read Error */
						Error_Handler();
					}
					else
					{
						HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"fopen OK\r", 9);
						osDelay(100);
						/*##-8- Read data from the text file ###########################*/
						res = f_read(&MyFile, rtext, sizeof(rtext), (void *)&bytesread);

						if((bytesread == 0) || (res != FR_OK))
						{
							if(bytesread == 0) {
								   HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"0 bytes read back error\r", 25);
								   osDelay(100);
							} else {
								   HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"fread result not ok\r", 20);
								   osDelay(100);
							}

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
								HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"Read file not correct\r", 22);
								osDelay(100);
								/* Read data is different from the expected data */
								Error_Handler();
							}
							else
							{
								HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"SD demo success\r", 16);
							    osDelay(100);

								/* Success of the demo: no error occurrence */
								bytesread = 0;
							}
						}
					}
				}
			}
		}
	}

	while(1){

		vTaskGetInfo( xTaskGetCurrentTaskHandle(), &task_3, 1, eRunning);
		osDelay(100);
	}

}

osThreadId buttonReadTaskHandle;
TaskStatus_t task_2;
GPIO_PinState button1_state, button2_state;
void buttonReadTask(void const * argument) {

	while(1) {
		if( HAL_GPIO_ReadPin(GPIOC, button_1_Pin) == GPIO_PIN_SET &&
			button1_state == GPIO_PIN_RESET) {
			button1_state = GPIO_PIN_SET;
			dsp_scrollTexboxRelative(&(g_textbox_list[0]), 0, 1);
			HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"Button1 released\r", 18);
		};
		if( HAL_GPIO_ReadPin(GPIOC, button_1_Pin) == GPIO_PIN_RESET &&
					button1_state == GPIO_PIN_SET) {
			button1_state = GPIO_PIN_RESET;
			HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"Button1 pressed\r", 17);
		};


		if( HAL_GPIO_ReadPin(GPIOC, button_2_Pin) == GPIO_PIN_SET &&
			button2_state == GPIO_PIN_RESET) {
			button2_state = GPIO_PIN_SET;


			HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"Button2 released\r", 18);
		};
		if( HAL_GPIO_ReadPin(GPIOC, button_2_Pin) == GPIO_PIN_RESET &&
					button2_state == GPIO_PIN_SET) {
			button2_state = GPIO_PIN_RESET;
			HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"Button2 pressed\r", 17);
		};

		if(button1_state == GPIO_PIN_RESET) { /*button pressed*/
			dsp_scrollTexboxRelative(&(g_textbox_list[0]), 0, 1);
		}

		if(button2_state == GPIO_PIN_RESET) { /*button pressed*/
			dsp_scrollTexboxRelative(&(g_textbox_list[1]), 1, 0);
		}
		WWDG_Refresh();

		vTaskGetInfo( xTaskGetCurrentTaskHandle(), &task_2, 1, eRunning);
		osDelay(10);

	}


}

uint8_t uartReadBuffer[100];
uint8_t puartReadBuffer=0;
uint8_t lineEndReceived=0;
void processMsg( void ) {
       int i;
       HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"Command received: ", 18);
       osDelay(100);
       HAL_UART_Transmit_IT(USART2_getHandle(), uartReadBuffer, puartReadBuffer);
       osDelay(100);

       	if(f_open(&MyFile, "cmd_log.TXT", FA_OPEN_ALWAYS | FA_WRITE) != FR_OK)
		{
			 HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"fopen not OK\r", 13);
				osDelay(100);
			/* 'STM32.TXT' file Open for write Error */
			Error_Handler();
		}
		else
		{
			 f_lseek(&MyFile, f_size(&MyFile));
			 HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"fopen OK\r", 9);
			 osDelay(100);

			 /*##-5- Write data to the text file ################################*/
			 res = f_write(&MyFile, uartReadBuffer, puartReadBuffer, (void *)&byteswritten);

			 if((byteswritten == 0) || (res != FR_OK))
			{
				 HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"fwrite not OK\r", 14);
					osDelay(100);

				/* 'STM32.TXT' file Write or EOF Error */
				Error_Handler();
			}
			else
			{
				HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"fwrite OK\r", 10);
				osDelay(100);
				/*##-6- Close the open text file #################################*/
				f_close(&MyFile);
				   HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"File closed!\r", 13);
				   osDelay(100);
			}
		}



       for(i=0;i<puartReadBuffer;i++){
               uartReadBuffer[i]=0;
       }
       puartReadBuffer=0;
}
void notifyReadTask(char data_in) {
       uartReadBuffer[puartReadBuffer] = data_in;
       puartReadBuffer++;
       if(data_in == '\n' || data_in == '\r') {
               lineEndReceived = 1;
       }
}
osThreadId uartReadTaskHandle;
TaskStatus_t task_1;
void uartReadTask(void const * argument) {

		HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"Nucleo alive!\r", 14);
		osDelay(1000);
		HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"wait!\r", 6);
		osDelay(10000);
		HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"Nucleo alive 2!\r", 16);
		osDelay(10000);

       while(1) {
               if(lineEndReceived) {
                       processMsg();
                       lineEndReceived = 0;
               }
               vTaskGetInfo( xTaskGetCurrentTaskHandle(), &task_1, 1, eRunning);
               osDelay(100);
       }

}





osThreadId displayTaskHandle;

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
  MX_SPI1_Init();
  MX_CRC_Init();
//  MX_TIM7_Init();
  MX_SDIO_SD_Init();
  /* Call init function for freertos objects (in freertos.c) */
    MX_FREERTOS_Init();
  /* USER CODE BEGIN 2 */



  /* USER CODE END 2 */

  osThreadDef(debug, debugTask, osPriorityNormal, 0, 400);
  debugTaskHandle = osThreadCreate(osThread(debug), NULL);

  osThreadDef(button, buttonReadTask, osPriorityNormal, 0, 200);
  buttonReadTaskHandle = osThreadCreate(osThread(button), NULL);

  osThreadDef(uart, uartReadTask, osPriorityNormal, 0, 300);
  uartReadTaskHandle = osThreadCreate(osThread(uart), NULL);


  osThreadDef(display, displayTask, osPriorityNormal, 0, 200);
  displayTaskHandle = osThreadCreate(osThread(display), NULL);

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
  //while(1)
  {
	  HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"Error\r", 6);
	  osDelay(10000);
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
