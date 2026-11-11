#ifndef LED_H
#define LED_H

#include <stdint.h>

typedef enum {
    LED_MODE_OFF = 0,
    LED_MODE_ON,
    LED_MODE_BLINK
} LED_Mode;

void LED_Init(void);
void LED_AllOn(void);
void LED_AllOff(void);
void LED_ToggleAll(void);
void LED_StartBlink(uint32_t now_ms);
uint8_t LED_Task(uint32_t now_ms);
uint8_t LED_IsOn(uint8_t index);
LED_Mode LED_GetMode(void);

#endif
