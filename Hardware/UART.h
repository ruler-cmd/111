#ifndef UART_H
#define UART_H

#include <stdint.h>

void UART_Init(void);

/*
 * Call from the main loop.  Parses one complete line received from USART1:
 *   "111"    -> set variable a = 111
 *   "-25"    -> set variable a = -25
 *   "GET A"  -> reply with the current value followed by CRLF
 */
void UART_Process(void);

int32_t UART_GetVarA(void);
uint8_t UART_VarChanged(void);

#ifdef UART_TEST
void UART_TestFeed(const char *line);
void UART_TestResetCapture(void);
uint16_t UART_TestGetCapture(uint8_t *destination, uint16_t max_length);
#endif

#endif
