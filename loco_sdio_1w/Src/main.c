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
#include "button.h"
#include "file_handler.h"
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
uint8_t text_1[] = "Logan Wheat went out on a small boat to check on cattle and ended up capturing one of the most startling photos of flooding from Harvey.\
		What used to be Interstate 10, south of Beaumont, Texas, looked like an ocean, with waves lapping.\
		This split screen shows what that stretch of I-10 looked like before Harvey hit -- and what it looks like now.\
		The wind-churned waves almost tipped Wheat's boat over, Wheat told CNN.\
		The boat was being thrown around a lot, he said.\
		The waves, he added, were anywhere from 3 to 4 feet.";
uint8_t text_2[] = "Houston (CNN)Harvey is no longer a hurricane, but life-threatening flooding continued in and around Houston on Sunday night as citizens with boats assisted authorities in search and rescue efforts.\
		Flooding from Tropical Storm Harvey is overburdening resources in the country's fourth-largest city, prompting authorities to call on volunteers with watercraft for help in rescuing those trapped in homes and buildings.\
		An immediate respite from Harvey's wrath seems unlikely to come. The National Weather Service calls the flooding and warns things may become more dire if a forecasted record-breaking 50 inches of rain does fall on parts of Texas in coming days. In anticipation of a worsening situation, Dallas is turning its main convention center into a mega-shelter that can host 5,000 evacuees.\
		The rainfall threatens to exacerbate an already dangerous situation, as Harvey's rains have left many east Texas rivers and bayous swollen to their banks or beyond.";
uint8_t used_text = 1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);
void MX_FREERTOS_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

void scrollDown(uint32_t duration) {
	dsp_scrollTexboxRelative(&(g_textbox_list[0]), 0, 2);
	dsp_scrollTexboxRelative(&(g_textbox_list[1]), 5, 0);
}
void scrollUp(uint32_t duration) {
	dsp_scrollTexboxRelative(&(g_textbox_list[0]), 0, -2);
	dsp_scrollTexboxRelative(&(g_textbox_list[1]), -5, 0);
}
void resetScroll(uint32_t duration) {
	dsp_scrollTexboxAbsolute(&(g_textbox_list[0]), 0, 0);
	dsp_scrollTexboxAbsolute(&(g_textbox_list[1]), 0, 0);
}
void nextText(uint32_t duration) {
	if(used_text == 1) {
		dsp_text_setText( &(g_textbox_list[0]),text_2, 0);
		dsp_text_setText( &(g_textbox_list[1]),text_2, 0);
		used_text = 2;
	} else if (used_text == 2) {
		dsp_text_setText( &(g_textbox_list[0]),text_1, 0);
		dsp_text_setText( &(g_textbox_list[1]),text_1, 0);
		used_text = 1;
	}

}
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */



uint8_t uartReadBuffer[100];
uint8_t puartReadBuffer=0;
uint8_t lineEndReceived=0;
int i,byteswritten;
FIL MyFile;
FRESULT res;
void processMsg( void ) {

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
		osDelay(1000);
		HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"Nucleo alive 2!\r", 16);
		osDelay(1000);

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
osThreadId debugTaskHandle;
osThreadId buttonReadTaskHandle;
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

    button_addEventHandler(BUTTON_1, EVENT_LONG_PRESS, scrollDown );
	button_addEventHandler(BUTTON_2, EVENT_LONG_PRESS, scrollUp );

	button_addEventHandler(BUTTON_1, EVENT_RELEASE, resetScroll );
	button_addEventHandler(BUTTON_2, EVENT_RELEASE, nextText );
	dsp_text_setText( &(g_textbox_list[0]),text_1, 0);
	dsp_text_setText( &(g_textbox_list[1]),text_1, 0);

  /* USER CODE END 2 */

  osThreadDef(uSD, fileHandlerTask, osPriorityNormal, 0, 500);
  debugTaskHandle = osThreadCreate(osThread(uSD), NULL);

  osThreadDef(button, buttonReadTask, osPriorityNormal, 0, 200);
  buttonReadTaskHandle = osThreadCreate(osThread(button), NULL);

  osThreadDef(uart, uartReadTask, osPriorityNormal, 0, 400);
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
	  osDelay(1000);
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
