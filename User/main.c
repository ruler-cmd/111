#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "Key.h"
#include "LED.h"
#include "menu.h"
#include "UART.h"

int main(void)
{
    OLED_Init();
    Key_Init();
    LED_Init();
    UART_Init();
    Menu_Init();

    /* Draw the power-on main menu immediately. */
    Menu_Refresh();

    while (1) {
        KeyCode key = Key_Scan();

        UART_Process();

        if (key != KEY_NONE) {
            Menu_HandleKey(key);
        }

        Menu_Tick();
        Menu_Refresh();
        Delay_ms(10);
    }
}
