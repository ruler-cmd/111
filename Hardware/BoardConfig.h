#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "stm32f10x.h"

/*
 * All hardware connections live here.  Change only this file when your
 * wiring differs from the default STM32F103C8T6 minimum-system board setup.
 *
 * Default wiring
 *   OLED SSD1306 (I2C) : SCL -> PB8, SDA -> PB9, VCC -> 3.3V, GND -> GND
 *   KEY1..KEY4         : PA0, PA4, PB12, PB13 -> GND (internal pull-up)
 *   LED1               : PC13 (on-board LED, active low)
 *   LED2               : PA1 (external LED, active high)
 */

/* ------------------------------- OLED I2C ------------------------------- */
#define OLED_SCL_PORT           GPIOB
#define OLED_SCL_PIN            GPIO_Pin_8
#define OLED_SDA_PORT           GPIOB
#define OLED_SDA_PIN            GPIO_Pin_9

/* 0x78 = 7-bit address 0x3C shifted left.  Use 0x7A for a 0x3D module. */
#define OLED_I2C_ADDRESS        0x78

/* -------------------------------- Buttons ------------------------------- */
#define KEY1_PORT               GPIOA
#define KEY1_PIN                GPIO_Pin_0
#define KEY2_PORT               GPIOA
#define KEY2_PIN                GPIO_Pin_4
#define KEY3_PORT               GPIOB
#define KEY3_PIN                GPIO_Pin_12
#define KEY4_PORT               GPIOB
#define KEY4_PIN                GPIO_Pin_13

/* 0: pressed = low (key to GND, internal pull-up).  1: pressed = high. */
#define KEY_ACTIVE_LEVEL        0

/* Number of consecutive 10 ms scans a new level must be stable. */
#define KEY_DEBOUNCE_SCANS      3

/* ---------------------------------- LEDs --------------------------------- */
/* LED1: on-board PC13, active low. */
#define LED1_PORT               GPIOC
#define LED1_PIN                GPIO_Pin_13
#define LED1_ACTIVE_LEVEL       0

/*
 * LED2: external LED on PA1.
 * Default wiring (active high):
 *   PA1 --- 330R --- LED anode, LED cathode --- GND
 * If your LED module is active low, change LED2_ACTIVE_LEVEL to 0.
 */
#define LED2_PORT               GPIOA
#define LED2_PIN                GPIO_Pin_1
#define LED2_ACTIVE_LEVEL       0

/* Alternating blink half-period in milliseconds. */
#define LED_BLINK_HALF_MS       500U

/* --------------------------------- UART1 --------------------------------- */
/* USART1: PA9 = TX, PA10 = RX.  Connect a USB-TTL adapter crosswise. */
#define UART_TX_PORT            GPIOA
#define UART_TX_PIN             GPIO_Pin_9
#define UART_RX_PORT            GPIOA
#define UART_RX_PIN             GPIO_Pin_10
#define UART_BAUDRATE           115200U

/* ------------------------------ Info page -------------------------------- */
/* Replace these two strings with your own pinyin name and student number. */
#define INFO_NAME               "LU Changheng"
#define INFO_STUDENT_ID         "32602364"

#endif
