/*
 * menu_handler.h
 *
 *  Created on: 2017. szept. 3.
 *      Author: Lenovo
 */

#ifndef MENU_HANDLER_H_
#define MENU_HANDLER_H_

#include <stdint.h>
typedef enum menuStates_e {
	MENU_START_PAGE,
	MENU_FILE_SELECT,
	MENU_READ_TEXT,
	MENU_SETTINGS
} menuStates_t;

typedef enum menuEvent_e {
	EVENT_NONE,
	EVENT_UP,
	EVENT_DOWN,
	EVENT_EXIT,
	EVENT_ENTER
} menuEvent_t;

void menu_showMenu(menuStates_t menu, uint8_t menu_changed);
void menu_logicStateMachine(menuEvent_t event);

void menu_upEvent(uint32_t duration);
void menu_downEvent(uint32_t duration);
void menu_enterEvent(uint32_t duration);
void menu_exitEvent(uint32_t duration);
#endif /* MENU_HANDLER_H_ */
