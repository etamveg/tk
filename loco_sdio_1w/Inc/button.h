/*
 * button.h
 *
 *  Created on: 2017. szept. 2.
 *      Author: Lenovo
 */

#ifndef BUTTON_H_
#define BUTTON_H_

#include <stdint.h>


typedef enum {
	BUTTON_RELEASED,
	BUTTON_JUST_RELEASED,
	BUTTON_PRESSED,
	BUTTON_JUST_PRESSED,
	BUTTON_PRESSED_LONG_PRESS_EVENT_SENT
} button_state_t;

typedef struct {
	button_state_t buttonState;
	uint32_t buttonStateTime;
} button_descriptor_t;

typedef void(*eventHandler_t)(uint32_t duration);

typedef struct buttonEventHandlers_s {
	eventHandler_t press;
	eventHandler_t long_press;
} buttonEventHandlers_t;

#define BUTTON_LONG_PRESS_TIME 	1000
#define BUTTON_1				1
#define BUTTON_2				2

#define EVENT_IDLE				0
#define EVENT_LONG_PRESS		1
#define EVENT_RELEASE			2
#define EVENT_DOUBLE_LONG_PRESS	3

void buttonReadTask(void const * argument);
void button_addEventHandler(uint8_t button_id, uint8_t event, eventHandler_t eventHandler );
void button_eventNotify(uint8_t button_id, uint8_t event, uint32_t duration);
#endif /* BUTTON_H_ */
