#include "Key.h"
#include "BoardConfig.h"

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
} KeyPin;

static const KeyPin s_keys[4] = {
    {KEY1_PORT, KEY1_PIN},
    {KEY2_PORT, KEY2_PIN},
    {KEY3_PORT, KEY3_PIN},
    {KEY4_PORT, KEY4_PIN},
};

/* stable[] is 1 while the debounced key is down, 0 while it is released. */
static uint8_t s_stable[4];
static uint8_t s_counter[4];

static uint8_t Key_IsDown(uint8_t index)
{
    uint8_t level = GPIO_ReadInputDataBit(s_keys[index].port, s_keys[index].pin) ? 1U : 0U;

#if KEY_ACTIVE_LEVEL == 0
    return (level == 0U) ? 1U : 0U;
#else
    return (level != 0U) ? 1U : 0U;
#endif
}

void Key_Init(void)
{
    GPIO_InitTypeDef gpio;
    uint8_t i;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
                           RCC_APB2Periph_GPIOC, ENABLE);

    gpio.GPIO_Mode = (KEY_ACTIVE_LEVEL == 0) ? GPIO_Mode_IPU : GPIO_Mode_IPD;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;

    for (i = 0; i < 4; i++) {
        gpio.GPIO_Pin = s_keys[i].pin;
        GPIO_Init(s_keys[i].port, &gpio);
        s_stable[i] = 0;
        s_counter[i] = 0;
    }
}

/*
 * Call every 10 ms.  Returns a key code only on the press edge, so holding a
 * key does not repeat the action.  The caller must not use a long blocking
 * delay, otherwise the debounce interval changes.
 */
KeyCode Key_Scan(void)
{
    KeyCode event = KEY_NONE;
    uint8_t i;

    for (i = 0; i < 4; i++) {
        uint8_t down = Key_IsDown(i);

        if (down != s_stable[i]) {
            if (++s_counter[i] >= KEY_DEBOUNCE_SCANS) {
                s_counter[i] = 0;
                s_stable[i] = down;
                if (down) {
                    event = (KeyCode)(i + 1);
                }
            }
        } else {
            s_counter[i] = 0;
        }
    }

    return event;
}
