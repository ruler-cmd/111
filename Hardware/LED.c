#include "LED.h"
#include "BoardConfig.h"

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
    uint8_t active_low;
} LedPin;

static const LedPin s_leds[2] = {
    {LED1_PORT, LED1_PIN, (LED1_ACTIVE_LEVEL == 0) ? 1U : 0U},
    {LED2_PORT, LED2_PIN, (LED2_ACTIVE_LEVEL == 0) ? 1U : 0U},
};

static uint8_t s_on[2];
static LED_Mode s_mode;
static uint8_t s_blink_phase;
static uint32_t s_last_toggle_ms;

static void Led_Apply(uint8_t index, uint8_t on)
{
    const LedPin *led = &s_leds[index];

    s_on[index] = on ? 1U : 0U;

    if (led->active_low) {
        if (s_on[index]) {
            GPIO_ResetBits(led->port, led->pin);
        } else {
            GPIO_SetBits(led->port, led->pin);
        }
    } else {
        if (s_on[index]) {
            GPIO_SetBits(led->port, led->pin);
        } else {
            GPIO_ResetBits(led->port, led->pin);
        }
    }
}

void LED_Init(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);

    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;

    gpio.GPIO_Pin = LED1_PIN;
    GPIO_Init(LED1_PORT, &gpio);
    gpio.GPIO_Pin = LED2_PIN;
    GPIO_Init(LED2_PORT, &gpio);

    s_mode = LED_MODE_OFF;
    s_blink_phase = 0;
    s_last_toggle_ms = 0;
    Led_Apply(0, 0);
    Led_Apply(1, 0);
}

void LED_AllOn(void)
{
    s_mode = LED_MODE_ON;
    Led_Apply(0, 1);
    Led_Apply(1, 1);
}

void LED_AllOff(void)
{
    s_mode = LED_MODE_OFF;
    s_blink_phase = 0;
    Led_Apply(0, 0);
    Led_Apply(1, 0);
}

/* KEY1 action: both off -> both on, any other state -> both off. */
void LED_ToggleAll(void)
{
    if (s_mode == LED_MODE_ON) {
        LED_AllOff();
    } else {
        LED_AllOn();
    }
}

/* KEY2 action: start immediately with LED1 on and LED2 off. */
void LED_StartBlink(uint32_t now_ms)
{
    s_mode = LED_MODE_BLINK;
    s_blink_phase = 1;
    s_last_toggle_ms = now_ms;

    Led_Apply(0, 1);
    Led_Apply(1, 0);
}

/*
 * Call every few milliseconds while blinking.  Returns 1 when the outputs
 * changed, so the menu can refresh the OLED immediately.
 */
uint8_t LED_Task(uint32_t now_ms)
{
    if (s_mode != LED_MODE_BLINK) {
        return 0;
    }

    if ((uint32_t)(now_ms - s_last_toggle_ms) >= LED_BLINK_HALF_MS) {
        s_last_toggle_ms = now_ms;
        s_blink_phase = (uint8_t)(s_blink_phase ? 0U : 1U);

        /* Always complementary: never both on or both off. */
        Led_Apply(0, s_blink_phase);
        Led_Apply(1, (uint8_t)(s_blink_phase ? 0U : 1U));
        return 1;
    }

    return 0;
}

uint8_t LED_IsOn(uint8_t index)
{
    if (index > 1U) {
        return 0;
    }
    return s_on[index];
}

LED_Mode LED_GetMode(void)
{
    return s_mode;
}
