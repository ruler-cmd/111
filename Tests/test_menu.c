#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "stm32f10x.h"
#include "Delay.h"
#include "menu.h"
#include "OLED.h"
#include "LED.h"

GPIO_TypeDef gpioa;
GPIO_TypeDef gpiob;
GPIO_TypeDef gpioc;

void RCC_APB2PeriphClockCmd(uint32_t peripheral, uint32_t enable)
{
    (void)peripheral;
    (void)enable;
}

void GPIO_Init(GPIO_TypeDef *port, GPIO_InitTypeDef *init)
{
    (void)port;
    (void)init;
}

void GPIO_SetBits(GPIO_TypeDef *port, uint16_t pin)
{
    (void)port;
    (void)pin;
}

void GPIO_ResetBits(GPIO_TypeDef *port, uint16_t pin)
{
    (void)port;
    (void)pin;
}

uint8_t GPIO_ReadInputDataBit(GPIO_TypeDef *port, uint16_t pin)
{
    (void)port;
    (void)pin;
    return 1; /* released, active-low keys */
}

void Delay_us(uint32_t microseconds)
{
    (void)microseconds;
}

void Delay_ms(uint32_t milliseconds)
{
    (void)milliseconds;
}

static void press(KeyCode key)
{
    Menu_HandleKey(key);
    Menu_Refresh();
}

int main(void)
{
    OLED_Init();
    Key_Init();
    LED_Init();
    Menu_Init();
    Menu_Refresh();

    /* a) power-on state */
    assert(Menu_TestGetPage() == PAGE_MAIN);
    assert(Menu_TestGetCursor() == 0);
    printf("PASS power-on: main menu, cursor 0\n");

    /* a) key2 down, key1 up, wrapping at the top */
    press(KEY2);
    assert(Menu_TestGetCursor() == 1);
    press(KEY2);
    assert(Menu_TestGetCursor() == 2);
    press(KEY1);
    assert(Menu_TestGetCursor() == 1);
    press(KEY1);
    assert(Menu_TestGetCursor() == 0);
    press(KEY1);
    assert(Menu_TestGetCursor() == 3);
    printf("PASS cursor move + wrap\n");

    /* b) enter LED, toggle it, return to main with the cursor kept */
    Menu_HandleKey(KEY2);
    Menu_HandleKey(KEY2);
    Menu_HandleKey(KEY2);
    assert(Menu_TestGetCursor() == 2);
    press(KEY3);
    assert(Menu_TestGetPage() == PAGE_LINE);
    press(KEY4);
    assert(Menu_TestGetPage() == PAGE_MAIN);
    assert(Menu_TestGetCursor() == 2);
    printf("PASS line submenu enter/back\n");

    Menu_HandleKey(KEY1);
    Menu_HandleKey(KEY1);
    assert(Menu_TestGetCursor() == 0);
    press(KEY3);
    assert(Menu_TestGetPage() == PAGE_LED);
    assert(LED_IsOn() == 0);
    press(KEY3);
    assert(LED_IsOn() == 1);
    press(KEY4);
    assert(Menu_TestGetPage() == PAGE_MAIN);
    printf("PASS LED submenu toggle/back\n");

    /* b) info submenu and a live uptime redraw */
    Menu_HandleKey(KEY2);
    press(KEY3);
    assert(Menu_TestGetPage() == PAGE_INFO);
    for (int i = 0; i < 100; i++) {
        Menu_Tick();
    }
    Menu_Refresh();
    press(KEY4);
    assert(Menu_TestGetPage() == PAGE_MAIN);
    printf("PASS info submenu/back\n");

    /* b) extra submenu counter */
    Menu_HandleKey(KEY2);
    Menu_HandleKey(KEY2);
    press(KEY3);
    assert(Menu_TestGetPage() == PAGE_EXTRA);
    press(KEY3);
    press(KEY3);
    assert(Menu_TestGetExtraCount() == 2);
    press(KEY4);
    assert(Menu_TestGetPage() == PAGE_MAIN);
    printf("PASS extra submenu counter/back\n");

    printf("ALL MENU TESTS PASSED\n");
    return 0;
}
