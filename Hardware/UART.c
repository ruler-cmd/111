#include "UART.h"
#include "BoardConfig.h"

#include <string.h>

#define UART_LINE_SIZE 40

static int32_t s_var_a;
static uint8_t s_var_dirty;

static volatile char s_line_buf[UART_LINE_SIZE];
static volatile uint8_t s_line_ready;

static char s_rx_buf[UART_LINE_SIZE];
static uint8_t s_rx_len;

#ifdef UART_TEST
static uint8_t s_tx_capture[128];
static uint16_t s_tx_len;
#endif

static void UART_SendChar(char ch)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) {
        /* wait for the transmit register */
    }
    USART_SendData(USART1, (uint16_t)(uint8_t)ch);

#ifdef UART_TEST
    if (s_tx_len < sizeof(s_tx_capture)) {
        s_tx_capture[s_tx_len++] = (uint8_t)ch;
    }
#endif
}

static void UART_SendString(const char *text)
{
    while (*text != '\0') {
        UART_SendChar(*text++);
    }
}

static void UART_SendInt(int32_t value)
{
    char digits[12];
    uint8_t count = 0;
    uint32_t magnitude;

    if (value < 0) {
        UART_SendChar('-');
        magnitude = (uint32_t)(-(value + 1)) + 1U;  /* handles INT32_MIN */
    } else {
        magnitude = (uint32_t)value;
    }

    do {
        digits[count++] = (char)('0' + (magnitude % 10U));
        magnitude /= 10U;
    } while (magnitude != 0U && count < sizeof(digits));

    while (count > 0U) {
        UART_SendChar(digits[--count]);
    }
}

static char UART_Upper(char ch)
{
    if (ch >= 'a' && ch <= 'z') {
        return (char)(ch - 'a' + 'A');
    }
    return ch;
}

static const char *UART_SkipSpaces(const char *text)
{
    while (*text == ' ' || *text == '\t') {
        text++;
    }
    return text;
}

/* Accepts "GET A", "get a", and extra spaces. */
static uint8_t UART_IsGetA(const char *text)
{
    const char *p = UART_SkipSpaces(text);

    if (UART_Upper(p[0]) != 'G' || UART_Upper(p[1]) != 'E' || UART_Upper(p[2]) != 'T') {
        return 0;
    }
    p += 3;
    if (*p != ' ' && *p != '\t') {
        return 0;
    }
    p = UART_SkipSpaces(p);
    if (UART_Upper(*p) != 'A') {
        return 0;
    }
    p++;
    p = UART_SkipSpaces(p);
    return (*p == '\0') ? 1U : 0U;
}

static uint8_t UART_ParseInt(const char *text, int32_t *result)
{
    const char *p = UART_SkipSpaces(text);
    int32_t value = 0;
    uint8_t negative = 0;
    uint8_t digits = 0;

    if (*p == '+' || *p == '-') {
        negative = (*p == '-') ? 1U : 0U;
        p++;
    }
    while (*p >= '0' && *p <= '9') {
        value = value * 10 + (int32_t)(*p - '0');
        p++;
        digits++;
    }
    p = UART_SkipSpaces(p);
    if (digits == 0U || *p != '\0') {
        return 0;
    }

    *result = negative ? -value : value;
    return 1;
}

void UART_Init(void)
{
    GPIO_InitTypeDef gpio;
    USART_InitTypeDef usart;
    NVIC_InitTypeDef nvic;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    gpio.GPIO_Pin = UART_TX_PIN;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(UART_TX_PORT, &gpio);

    gpio.GPIO_Pin = UART_RX_PIN;
    gpio.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(UART_RX_PORT, &gpio);

    usart.USART_BaudRate = UART_BAUDRATE;
    usart.USART_WordLength = USART_WordLength_8b;
    usart.USART_StopBits = USART_StopBits_1;
    usart.USART_Parity = USART_Parity_No;
    usart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    usart.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &usart);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    nvic.NVIC_IRQChannel = USART1_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 1;
    nvic.NVIC_IRQChannelSubPriority = 1;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    USART_Cmd(USART1, ENABLE);

    s_var_a = 0;
    s_var_dirty = 0;
    s_line_ready = 0;
    s_rx_len = 0;
}

void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        char ch = (char)USART_ReceiveData(USART1);

        if (ch == '\r' || ch == '\n') {
            if (s_rx_len > 0U) {
                s_rx_buf[s_rx_len] = '\0';
                memcpy((char *)s_line_buf, s_rx_buf, (size_t)s_rx_len + 1U);
                s_line_ready = 1;
                s_rx_len = 0;
            }
        } else if (s_rx_len < (UART_LINE_SIZE - 1U)) {
            s_rx_buf[s_rx_len++] = ch;
        }
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

void UART_Process(void)
{
    char line[UART_LINE_SIZE];
    int32_t value;

    if (!s_line_ready) {
        return;
    }

    __disable_irq();
    memcpy(line, (const char *)s_line_buf, sizeof(line));
    s_line_ready = 0;
    __enable_irq();

    line[UART_LINE_SIZE - 1U] = '\0';

    if (UART_IsGetA(line)) {
        UART_SendInt(s_var_a);
        UART_SendString("\r\n");
        return;
    }

    if (UART_ParseInt(line, &value)) {
        s_var_a = value;
        s_var_dirty = 1;
    }
}

int32_t UART_GetVarA(void)
{
    return s_var_a;
}

uint8_t UART_VarChanged(void)
{
    uint8_t changed = s_var_dirty;
    s_var_dirty = 0;
    return changed;
}

#ifdef UART_TEST
void UART_TestFeed(const char *line)
{
    size_t length = strlen(line);

    if (length >= UART_LINE_SIZE) {
        length = UART_LINE_SIZE - 1U;
    }
    memcpy((char *)s_line_buf, line, length);
    ((char *)s_line_buf)[length] = '\0';
    s_line_ready = 1;
}

void UART_TestResetCapture(void)
{
    s_tx_len = 0;
}

uint16_t UART_TestGetCapture(uint8_t *destination, uint16_t max_length)
{
    uint16_t length = (s_tx_len < max_length) ? s_tx_len : max_length;

    memcpy(destination, s_tx_capture, length);
    return length;
}
#endif
