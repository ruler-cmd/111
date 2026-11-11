#ifndef MENU_H
#define MENU_H

#include <stdint.h>
#include "Key.h"

typedef enum {
    PAGE_MAIN = 0,
    PAGE_LED,
    PAGE_INFO,
    PAGE_LINE,
    PAGE_EXTRA
} MenuPage;

void Menu_Init(void);
void Menu_HandleKey(KeyCode key);
void Menu_Tick(void);
void Menu_Refresh(void);

#ifdef MENU_TEST
MenuPage Menu_TestGetPage(void);
uint8_t Menu_TestGetCursor(void);
uint16_t Menu_TestGetExtraCount(void);
#endif

#endif
