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
    return 1;
}

void Delay_us(uint32_t microseconds)
{
    (void)microseconds;
}

void Delay_ms(uint32_t milliseconds)
{
    (void)milliseconds;
}

static void dump(const char *path)
{
    uint8_t gram[1024];
    FILE *file;

    OLED_TestCopyGram(gram);
    file = fopen(path, "wb");
    if (file == NULL) {
        perror(path);
        return;
    }
    fwrite(gram, 1, sizeof(gram), file);
    fclose(file);
}

static void key(KeyCode code)
{
    Menu_HandleKey(code);
    Menu_Refresh();
}

int main(void)
{
    OLED_Init();
    Key_Init();
    LED_Init();
    Menu_Init();
    Menu_Refresh();

    dump("tests/out/main0.raw");
    key(KEY2);
    dump("tests/out/main1.raw");

    key(KEY1);
    key(KEY3);
    dump("tests/out/led.raw");
    key(KEY3);
    dump("tests/out/led_on.raw");
    key(KEY4);

    key(KEY2);
    key(KEY3);
    for (int i = 0; i < 1200; i++) {
        Menu_Tick();
    }
    Menu_Refresh();
    dump("tests/out/info.raw");
    key(KEY4);

    key(KEY2);
    key(KEY3);
    dump("tests/out/line.raw");
    key(KEY4);

    key(KEY2);
    key(KEY3);
    key(KEY3);
    key(KEY3);
    dump("tests/out/extra.raw");

    return 0;
}
