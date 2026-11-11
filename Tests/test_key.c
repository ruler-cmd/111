#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "stm32f10x.h"
#include "Key.h"

GPIO_TypeDef gpioa;
GPIO_TypeDef gpiob;
GPIO_TypeDef gpioc;

static uint8_t s_level[4] = {1, 1, 1, 1}; /* released = high */

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
    switch (pin) {
        case GPIO_Pin_0:  return s_level[0];
        case GPIO_Pin_1:  return s_level[1];
        case GPIO_Pin_10: return s_level[2];
        case GPIO_Pin_11: return s_level[3];
        default:          return 1;
    }
}

static void scans(unsigned count, KeyCode expected)
{
    for (unsigned i = 0; i < count; i++) {
        KeyCode key = Key_Scan();
        if (i + 1 == count) {
            assert(key == expected);
        } else {
            assert(key == KEY_NONE);
        }
    }
}

static void test_key(unsigned index, KeyCode code)
{
    /* press: two scans are not enough, the third one produces the edge */
    s_level[index] = 0;
    scans(2, KEY_NONE);
    scans(1, code);
    /* held down: no repeat */
    scans(5, KEY_NONE);
    /* release: needs debounce too */
    s_level[index] = 1;
    scans(3, KEY_NONE);
    /* press again: a new edge */
    s_level[index] = 0;
    scans(3, code);
    s_level[index] = 1;
    scans(3, KEY_NONE);
}

int main(void)
{
    Key_Init();
    test_key(0, KEY1);
    test_key(1, KEY2);
    test_key(2, KEY3);
    test_key(3, KEY4);
    printf("PASS key debounce / one-shot edge / all 4 keys\n");
    return 0;
}
