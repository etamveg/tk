/*
 * button.c
 *
 *  Created on: 2017. szept. 2.
 *      Author: Lenovo
 */

#include "button.h"
#include "cmsis_os.h"
#include "main.h"
#include "stm32f4xx_hal.h"
void buttonDummyHandler(uint32_t duration);

button_descriptor_t button1, button2;
buttonEventHandlers_t buttons[2];
TaskStatus_t task_2;
void buttonReadTask(void const * argument) {
	uint8_t buttonReadPeriod = 50;

	while(1) {
		if( HAL_GPIO_ReadPin(GPIOC, button_1_Pin) == GPIO_PIN_SET &&
				button1.buttonState == BUTTON_PRESSED) {
			button1.buttonState = BUTTON_JUST_RELEASED;
			HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"Button1 released\r", 18);

		};
		if( HAL_GPIO_ReadPin(GPIOC, button_1_Pin) == GPIO_PIN_RESET &&
				button1.buttonState == BUTTON_RELEASED) {
			button1.buttonState = BUTTON_JUST_PRESSED;
			HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"Button1 pressed\r", 17);
		};


		if( HAL_GPIO_ReadPin(GPIOC, button_2_Pin) == GPIO_PIN_SET &&
				button2.buttonState == BUTTON_PRESSED) {
			button2.buttonState = BUTTON_JUST_RELEASED;
			HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"Button2 released\r", 18);
		};
		if( HAL_GPIO_ReadPin(GPIOC, button_2_Pin) == GPIO_PIN_RESET &&
				button2.buttonState == BUTTON_RELEASED) {
			button2.buttonState = BUTTON_JUST_PRESSED;
			HAL_UART_Transmit_IT(USART2_getHandle(), (uint8_t*)"Button2 pressed\r", 17);
		};

		button1.buttonStateTime += buttonReadPeriod;
		button2.buttonStateTime += buttonReadPeriod;

		if(button1.buttonState == BUTTON_JUST_RELEASED ) {
			button1.buttonState = BUTTON_RELEASED;
			if(button1.buttonStateTime < BUTTON_LONG_PRESS_TIME) {
				button_eventNotify(BUTTON_1, EVENT_RELEASE, button1.buttonStateTime);
			}
			button1.buttonStateTime = 0;
		} else if(button1.buttonState == BUTTON_RELEASED ) {
			;
		} else if(button1.buttonState == BUTTON_JUST_PRESSED ) {
			button1.buttonState = BUTTON_PRESSED;

			button1.buttonStateTime = 0;
		} else if(button1.buttonState == BUTTON_PRESSED ) {
			if(button1.buttonStateTime > BUTTON_LONG_PRESS_TIME)
				button_eventNotify(BUTTON_1, EVENT_LONG_PRESS, button1.buttonStateTime);
		}

		if(button2.buttonState == BUTTON_JUST_RELEASED ) {
			button2.buttonState = BUTTON_RELEASED;
			if(button2.buttonStateTime < BUTTON_LONG_PRESS_TIME) {
				button_eventNotify(BUTTON_2, EVENT_RELEASE, button2.buttonStateTime);
			}
			button2.buttonStateTime = 0;
		} else if(button2.buttonState == BUTTON_RELEASED ) {
			;
		} else if(button2.buttonState == BUTTON_JUST_PRESSED ) {
			button2.buttonState = BUTTON_PRESSED;

			button2.buttonStateTime = 0;
		} else if(button2.buttonState == BUTTON_PRESSED ) {
			if(button2.buttonStateTime > BUTTON_LONG_PRESS_TIME)
				button_eventNotify(BUTTON_2, EVENT_LONG_PRESS, button2.buttonStateTime);
		}

		WWDG_Refresh();

		vTaskGetInfo( xTaskGetCurrentTaskHandle(), &task_2, 1, eRunning);
		osDelay(buttonReadPeriod);

	}


}



buttonEventHandlers_t buttons[2];

void button_eventNotify(uint8_t button_id, uint8_t event, uint32_t duration) {
	switch(button_id) {
	case BUTTON_1:
		if(event == EVENT_RELEASE) {
			if(buttons[0].press != NULL) {
				buttons[0].press(duration);
			}
		} else if(event == EVENT_LONG_PRESS) {
			if(buttons[0].long_press != NULL) {
				buttons[0].long_press(duration);
			}
		}
		break;
	case BUTTON_2:
		if(event == EVENT_RELEASE) {
			if(buttons[1].press != NULL) {
				buttons[1].press(duration);
			}
		} else if(event == EVENT_LONG_PRESS) {
			if(buttons[1].long_press != NULL) {
				buttons[1].long_press(duration);
			}
		}
		break;
	}
}

void button_addEventHandler(uint8_t button_id, uint8_t event, eventHandler_t eventHandler ) {
	switch(button_id) {
	case BUTTON_1:
		if(event == EVENT_RELEASE) {
			buttons[0].press = eventHandler;
		} else if(event == EVENT_LONG_PRESS) {
			buttons[0].long_press = eventHandler;
		}
		break;
	case BUTTON_2:
		if(event == EVENT_RELEASE) {
			buttons[1].press = eventHandler;
		} else if(event == EVENT_LONG_PRESS) {
			buttons[1].long_press = eventHandler;
		}
		break;
	}
}

void buttonDummyHandler(uint32_t duration){
	;
}
