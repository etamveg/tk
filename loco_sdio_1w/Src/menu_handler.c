/*
 * menu_handler.c
 *
 *  Created on: 2017. szept. 3.
 *      Author: Lenovo
 */

#include <stdlib.h>
#include <string.h>
#include "menu_handler.h"
#include "display.h"
#include "file_handler.h"
#include "cmsis_os.h"

uint8_t menu_item_selector = 0;

dsp_textbox_t item_select;
char item_select_text[10];
char textContent[300];

dsp_textbox_t start_page_a, start_page_b, start_page_c;
char start_page_file_select[] = "File select";
char start_page_file_read[] 	= "File read";
char start_page_settings[] 	= "Settings";
char file_to_open[24];
uint8_t fileRequestState;
uint8_t readTextFontSize = 8;
uint8_t folder_changed = 0;

void menu_showMenu(menuStates_t menu, uint8_t menu_changed) {
	dsp_cleanDisplayData(displayData);
	if(menu == MENU_START_PAGE) {
		if(menu_changed) {
			dsp_text_DeleteTextbox(&start_page_a);
			dsp_text_DeleteTextbox(&start_page_b);
			dsp_text_DeleteTextbox(&start_page_c);
			dsp_text_DeleteTextbox(&item_select);
			dsp_text_InitTextbox( &start_page_a, 12, 80,   3, 10, 12, 5 );
			dsp_text_InitTextbox( &start_page_b, 12, 80,  88, 10, 12, 5 );
			dsp_text_InitTextbox( &start_page_c, 12, 80, 173, 10, 12, 5 );
			dsp_text_InitTextbox( &item_select, 12, 80, 3, 22, 12, 5 );

			dsp_text_setText( &start_page_a,(uint8_t*)start_page_file_select, 0);
			dsp_text_setText( &start_page_b,(uint8_t*)start_page_file_read, 0);
			dsp_text_setText( &start_page_c,(uint8_t*)start_page_settings, 0);
			strcpy(item_select_text, "xxxxxxxxxx");
			dsp_text_setText( &item_select,(uint8_t*)item_select_text, 0);

			dsp_fillTextBoxWithText(&start_page_a);
			dsp_fillTextBoxWithText(&start_page_b);
			dsp_fillTextBoxWithText(&start_page_c);
			dsp_fillTextBoxWithText(&item_select);
		}

		dsp_setTbInversion(&start_page_a, 0);
		dsp_setTbInversion(&start_page_b, 0);
		dsp_setTbInversion(&start_page_c, 0);

		if(menu_item_selector == 0 ){
			dsp_setTbPosition(&item_select,3, 22);
			dsp_setTbInversion(&start_page_a, 1);
		} else if(menu_item_selector == 1 ){
			dsp_setTbPosition(&item_select,88, 22);
			dsp_setTbInversion(&start_page_b, 1);
		} else if(menu_item_selector == 2 ){
			dsp_setTbPosition(&item_select,173, 22);
			dsp_setTbInversion(&start_page_c, 1);
		}


		dsp_txt_printTBToMemory(&start_page_a, displayData);
		dsp_txt_printTBToMemory(&start_page_b, displayData);
		dsp_txt_printTBToMemory(&start_page_c, displayData);
		dsp_txt_printTBToMemory(&item_select, displayData);
	} else if(menu == MENU_FILE_SELECT) {
		if(folder_changed){
			folder_changed=0;
			uint8_t request;
			while(file_refreshDirectoryContent(&request))  osDelay(10);
			while(request != 1) osDelay(10);
		}


		if(menu_changed) {
			dsp_text_DeleteTextbox(&start_page_a);
			dsp_text_DeleteTextbox(&start_page_b);
			dsp_text_DeleteTextbox(&item_select);
			dsp_text_InitTextbox( &start_page_a, 32, 72,   0, 0, 8, 5 );
			dsp_text_InitTextbox( &start_page_b, 32, 120,   80, 0, 8, 5 );
			dsp_text_InitTextbox( &item_select, 32, 30,  210, 0, 8, 5 );

			/*Create data*/
			//path
			char *ptext;
			uint32_t len;
			file_getCurrentPath(&ptext);
			dsp_text_setText( &start_page_a,(uint8_t *)ptext, 0);


			file_getDirectoryContent(&ptext, &len);
			//parse
			int i=0, j=0, endDetect=0;
			for(i=0; i<300; i++){
				if(ptext[j] == '\n') {
					endDetect=1;
					j++;
				}
				if(i%24 == 0) endDetect = 0;
				if(endDetect){
					if(i%24 == 23)
						textContent[i]='\n';
					else
						textContent[i]=' ';
				} else {
					textContent[i]=ptext[j];
					j++;
				}
				if(j>=len || ptext[j] == '\0') {
					break;
				}
			}

			dsp_text_setText( &start_page_b,(uint8_t*)textContent, 0);
			strcpy(item_select_text, "<--");
			dsp_text_setText( &item_select,(uint8_t*)item_select_text, 0);


		}

		if(menu_item_selector>2){
			start_page_b.lineOffset_px = (menu_item_selector-2) * 8;
		} else {
			start_page_b.lineOffset_px = 0;
		}

		dsp_fillTextBoxWithText(&start_page_a);
		dsp_fillTextBoxWithText(&start_page_b);
		dsp_fillTextBoxWithText(&item_select);

		if(menu_item_selector == 0 ){
			dsp_setTbPosition(&item_select,210, 0);
		} else if(menu_item_selector == 1 ){
			dsp_setTbPosition(&item_select,210, 8);
		} else if(menu_item_selector > 1 ){
			dsp_setTbPosition(&item_select,210, 16);
		}
		dsp_txt_printTBToMemory(&start_page_a, displayData);
		dsp_txt_printTBToMemory(&start_page_b, displayData);
		dsp_txt_printTBToMemory(&item_select, displayData);
	} else if(menu == MENU_READ_TEXT) {
		if(menu_changed) {
			dsp_text_DeleteTextbox(&start_page_a);
			uint32_t len;
			char *ptext;
			dsp_text_InitTextbox( &start_page_a, 32, 250,   0, 0, readTextFontSize, 5 );
			if(strlen(file_to_open) > 1) {
				while(file_readTextRequest((uint8_t*)file_to_open, 0, 300,&fileRequestState))  osDelay(100);
				while(fileRequestState != 1) osDelay(100);
				file_getFileContent(&ptext, &len);
				dsp_text_setText( &start_page_a,(uint8_t*)ptext, 0);
			} else {
				dsp_text_setText( &start_page_a,(uint8_t*)"Please select a file to open!", 0);
			}
		}
		dsp_fillTextBoxWithText(&start_page_a);
		dsp_txt_printTBToMemory(&start_page_a, displayData);
	} else if(menu == MENU_SETTINGS) {
		if(menu_changed) {
			dsp_text_DeleteTextbox(&item_select);

			dsp_text_InitTextbox( &item_select, 8, 8, 121, 0, 8, 5 );
			strcpy(item_select_text, "<");
			dsp_text_setText( &item_select,(uint8_t*)item_select_text, 0);
			dsp_fillTextBoxWithText(&item_select);
		}
		dsp_printLineToMemory(displayData, 8, 0, 0, (uint8_t*)"Settings", 8,256, 32, 0);

		dsp_printLineToMemory(displayData, 8, 40, 8, (uint8_t*)"Set font", 8,256, 32, 0);
		dsp_printLineToMemory(displayData, 8, 175, 8, (uint8_t*)"  8x5 ", 8,256, 32, menu_item_selector==0);
		dsp_printLineToMemory(displayData, 8, 175, 16, (uint8_t*)" 12x8 ", 8,256, 32, menu_item_selector==1);
		dsp_printLineToMemory(displayData, 8, 175, 24, (uint8_t*)"16x10 ", 8,256, 32, menu_item_selector==2);
		if(menu_item_selector == 0 ){
			dsp_setTbPosition(&item_select,210, 8);
		} else if(menu_item_selector == 1 ){
			dsp_setTbPosition(&item_select,210, 16);
		} else if(menu_item_selector > 1 ){
			dsp_setTbPosition(&item_select,210, 24);
		}

		dsp_txt_printTBToMemory(&item_select, displayData);
	}
}

menuStates_t state;
void menu_logicStateMachine(menuEvent_t event) {
	static uint8_t firstRun = 1;
	uint8_t menu_changed = 0;
	if(firstRun){
		firstRun = 0;
		state = MENU_START_PAGE;
		menu_changed=1;
	}

	switch(state){
		case MENU_START_PAGE:
			switch(event){
				case EVENT_UP:
					menu_item_selector = (menu_item_selector+1)%3;
					break;
				case EVENT_DOWN:
					if(menu_item_selector) menu_item_selector = (menu_item_selector-1)%3;
					break;
				case EVENT_EXIT:
					break;
				case EVENT_ENTER:
					menu_changed = 1;
					switch(menu_item_selector) {
					case 0:
						state = MENU_FILE_SELECT;
						//directory content refresh
						while(file_refreshDirectoryContent(&fileRequestState))  osDelay(10);
						while(fileRequestState != 1) osDelay(10);
						menu_item_selector=0;
						break;
					case 1:
						state = MENU_READ_TEXT;
						menu_item_selector=0;
						break;
					case 2:
						state = MENU_SETTINGS;
						menu_item_selector=0;
						break;
					}
					dsp_text_DeleteTextbox(&start_page_a);
					dsp_text_DeleteTextbox(&start_page_b);
					dsp_text_DeleteTextbox(&start_page_c);
					dsp_text_DeleteTextbox(&item_select);
					break;
				case EVENT_NONE:
				default:
					break;

			}
			break;
		case MENU_FILE_SELECT:
			switch(event){
				case EVENT_UP:
					menu_item_selector = (menu_item_selector+1);
					break;
				case EVENT_DOWN:
					if(menu_item_selector) menu_item_selector = (menu_item_selector-1);
					break;
				case EVENT_EXIT:
					menu_changed = 1;
					dsp_text_DeleteTextbox(&start_page_a);
					dsp_text_DeleteTextbox(&start_page_b);
					dsp_text_DeleteTextbox(&item_select);
					state = MENU_START_PAGE;
					break;
				case EVENT_ENTER:
					{
						uint8_t line_number = 0;
						uint8_t request = 0;
						char *pLine;
						char *pSpace;
						pLine = (char*)start_page_b.textData_char;

						while(line_number < menu_item_selector)
						{
							line_number++;
							pLine = strchr(pLine, '\n');
							pLine++;
						}
						pSpace = strchr(pLine, ' ');
						*pSpace = '\0';
						strcpy(file_to_open, pLine);
						menu_changed = 1;

						if(pSpace[-1]=='.' &&pSpace[-2]=='.' && pSpace[-3]=='.') {
							dsp_text_DeleteTextbox(&start_page_a);
							dsp_text_DeleteTextbox(&start_page_b);
							dsp_text_DeleteTextbox(&item_select);
							pSpace[-3]='\0';
							while(file_enterDirectory(pLine, &request) ) osDelay(10);
							while(request != 1) osDelay(10);
//							while(file_refreshDirectoryContent(&request))  osDelay(10);
//							while(request != 1) osDelay(10);
							folder_changed = 1;

						} else {
							dsp_text_DeleteTextbox(&start_page_a);
							dsp_text_DeleteTextbox(&start_page_b);
							dsp_text_DeleteTextbox(&item_select);
							state = MENU_READ_TEXT;
						}

					}
					break;
				case EVENT_NONE:
				default:
					break;

			}
			break;
		case MENU_READ_TEXT:
			switch(event){
				case EVENT_UP:
					if((int)start_page_a.lineOffset_px - 4 > 0){
						start_page_a.lineOffset_px -= 4;
					} else {
						start_page_a.lineOffset_px = 0;
					}
					break;
				case EVENT_DOWN:
					start_page_a.lineOffset_px += 4;
					break;
				case EVENT_EXIT:
					menu_changed = 1;
					dsp_text_DeleteTextbox(&start_page_a);
					state = MENU_START_PAGE;
					break;
				case EVENT_ENTER:
					break;
				case EVENT_NONE:
				default:
					break;

			}
			break;
		case MENU_SETTINGS:
			switch(event){
				case EVENT_UP:
					menu_item_selector = (menu_item_selector+1)%3;
					break;
				case EVENT_DOWN:
					if(menu_item_selector) menu_item_selector = (menu_item_selector-1)%3;
					break;
				case EVENT_EXIT:
					menu_changed = 1;
					dsp_text_DeleteTextbox(&item_select);
					state = MENU_START_PAGE;
					break;
				case EVENT_ENTER:
					if(menu_item_selector == 0) readTextFontSize = 8;
					if(menu_item_selector == 1) readTextFontSize = 12;
					if(menu_item_selector == 2) readTextFontSize = 16;
					break;
				case EVENT_NONE:
				default:
					break;

			}
			break;
	}

	menu_showMenu(state,menu_changed);


}

void menu_upEvent(uint32_t duration) {
	menu_logicStateMachine(EVENT_UP);
}
void menu_downEvent(uint32_t duration) {
	menu_logicStateMachine(EVENT_DOWN);
}
void menu_enterEvent(uint32_t duration) {
	menu_logicStateMachine(EVENT_ENTER);
}
void menu_exitEvent(uint32_t duration) {
	menu_logicStateMachine(EVENT_EXIT);
}
