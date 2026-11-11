#include "OLED.h"
#include "OLED_Font.h"
#include "BoardConfig.h"
#include "Delay.h"

#include <string.h>

#define SCL_H()     GPIO_SetBits(OLED_SCL_PORT, OLED_SCL_PIN)
#define SCL_L()     GPIO_ResetBits(OLED_SCL_PORT, OLED_SCL_PIN)
#define SDA_H()     GPIO_SetBits(OLED_SDA_PORT, OLED_SDA_PIN)
#define SDA_L()     GPIO_ResetBits(OLED_SDA_PORT, OLED_SDA_PIN)
#define SDA_READ()  GPIO_ReadInputDataBit(OLED_SDA_PORT, OLED_SDA_PIN)

static uint8_t s_gram[8][OLED_WIDTH];

/* One microsecond per half bit gives roughly 300-400 kHz on a 72 MHz part. */
static void I2C_Delay(void)
{
    Delay_us(1);
}

static void I2C_InitPin(GPIO_TypeDef *port, uint16_t pin)
{
    GPIO_InitTypeDef gpio;

    gpio.GPIO_Mode = GPIO_Mode_Out_OD;
    gpio.GPIO_Pin = pin;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(port, &gpio);
}

static void I2C_Start(void)
{
    SDA_H();
    SCL_H();
    I2C_Delay();
    SDA_L();
    I2C_Delay();
    SCL_L();
    I2C_Delay();
}

static void I2C_Stop(void)
{
    SDA_L();
    I2C_Delay();
    SCL_H();
    I2C_Delay();
    SDA_H();
    I2C_Delay();
}

static uint8_t I2C_WriteByte(uint8_t value)
{
    uint8_t i;
    uint8_t ack;

    for (i = 0; i < 8; i++) {
        if (value & 0x80U) {
            SDA_H();
        } else {
            SDA_L();
        }
        value <<= 1;
        I2C_Delay();
        SCL_H();
        I2C_Delay();
        SCL_L();
        I2C_Delay();
    }

    /* Release SDA and read the acknowledge bit. */
    SDA_H();
    I2C_Delay();
    SCL_H();
    I2C_Delay();
    ack = SDA_READ() ? 0U : 1U;
    SCL_L();
    I2C_Delay();

    return ack;
}

static void OLED_WriteCommand(uint8_t command)
{
    I2C_Start();
    I2C_WriteByte(OLED_I2C_ADDRESS);
    I2C_WriteByte(0x00);              /* Co = 0, D/C# = 0: command */
    I2C_WriteByte(command);
    I2C_Stop();
}

static void OLED_WriteData(const uint8_t *data, uint16_t length)
{
    uint16_t i;

    I2C_Start();
    I2C_WriteByte(OLED_I2C_ADDRESS);
    I2C_WriteByte(0x40);              /* Co = 0, D/C# = 1: data */
    for (i = 0; i < length; i++) {
        I2C_WriteByte(data[i]);
    }
    I2C_Stop();
}

void OLED_Init(void)
{
    static const uint8_t init_commands[] = {
        0xAE,             /* display off */
        0xD5, 0x80,       /* clock divide / oscillator */
        0xA8, 0x3F,       /* multiplex = 64 */
        0xD3, 0x00,       /* display offset = 0 */
        0x40,             /* start line = 0 */
        0x8D, 0x14,       /* charge pump on */
        0x20, 0x02,       /* page addressing mode */
        0xA1,             /* segment remap */
        0xC8,             /* COM scan direction remapped */
        0xDA, 0x12,       /* COM pins */
        0x81, 0xCF,       /* contrast */
        0xD9, 0xF1,       /* pre-charge period */
        0xDB, 0x40,       /* VCOMH deselect level */
        0xA4,             /* display follows RAM */
        0xA6,             /* normal (not inverted) */
        0x2E              /* deactivate scroll */
    };
    uint8_t i;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
                           RCC_APB2Periph_GPIOC, ENABLE);
    I2C_InitPin(OLED_SCL_PORT, OLED_SCL_PIN);
    I2C_InitPin(OLED_SDA_PORT, OLED_SDA_PIN);
    SCL_H();
    SDA_H();

    Delay_ms(100);                    /* let the panel power up */

    for (i = 0; i < sizeof(init_commands); i++) {
        OLED_WriteCommand(init_commands[i]);
    }

    OLED_Clear();
    OLED_Refresh();
    OLED_WriteCommand(0xAF);          /* display on after RAM is clean */
}

void OLED_Clear(void)
{
    memset(s_gram, 0x00, sizeof(s_gram));
}

void OLED_Refresh(void)
{
    uint8_t page;

    for (page = 0; page < 8; page++) {
        OLED_WriteCommand((uint8_t)(0xB0U | page));
        OLED_WriteCommand(0x00);      /* lower column = 0 */
        OLED_WriteCommand(0x10);      /* higher column = 0 */
        OLED_WriteData(&s_gram[page][0], OLED_WIDTH);
    }
}

void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT) {
        return;
    }

    if (color) {
        s_gram[y / 8][x] |= (uint8_t)(1U << (y % 8));
    } else {
        s_gram[y / 8][x] &= (uint8_t)~(1U << (y % 8));
    }
}

void OLED_FillRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color)
{
    uint8_t xx;
    uint8_t yy;

    for (yy = 0; yy < height; yy++) {
        for (xx = 0; xx < width; xx++) {
            OLED_DrawPixel((uint8_t)(x + xx), (uint8_t)(y + yy), color);
        }
    }
}

void OLED_DrawHLine(uint8_t x, uint8_t y, uint8_t width, uint8_t color)
{
    uint8_t xx;

    for (xx = 0; xx < width; xx++) {
        OLED_DrawPixel((uint8_t)(x + xx), y, color);
    }
}

/* A 7x7 filled triangle, easier to see than a '>' character. */
void OLED_DrawArrow(uint8_t x, uint8_t y, uint8_t color)
{
    static const uint8_t arrow[7] = {0x40, 0x60, 0x70, 0x78, 0x70, 0x60, 0x40};
    uint8_t col;
    uint8_t row;
    uint8_t background = color ? 0U : 1U;

    for (col = 0; col < 7; col++) {
        for (row = 0; row < 7; row++) {
            OLED_DrawPixel((uint8_t)(x + col), (uint8_t)(y + row),
                           (arrow[col] & (uint8_t)(1U << row)) ? color : background);
        }
    }
}

void OLED_ShowChar(uint8_t x, uint8_t y, char ch, uint8_t color)
{
    const uint8_t *glyph;
    uint8_t col;
    uint8_t row;
    uint8_t background = color ? 0U : 1U;

    if (ch < 0x20 || ch > 0x7E) {
        ch = '?';
    }
    glyph = OLED_Font6x8[(uint8_t)ch - 0x20U];

    for (col = 0; col < 6; col++) {
        for (row = 0; row < 8; row++) {
            OLED_DrawPixel((uint8_t)(x + col), (uint8_t)(y + row),
                           (glyph[col] & (uint8_t)(1U << row)) ? color : background);
        }
    }
}

void OLED_ShowString(uint8_t x, uint8_t y, const char *text, uint8_t color)
{
    while (*text != '\0' && x <= (OLED_WIDTH - 6)) {
        OLED_ShowChar(x, y, *text, color);
        x = (uint8_t)(x + 6);
        text++;
    }
}

void OLED_ShowUnsigned(uint8_t x, uint8_t y, uint32_t value, uint8_t digits, uint8_t color)
{
    uint8_t i;
    uint8_t started = 0;

    if (digits == 0 || digits > 10) {
        return;
    }

    for (i = 0; i < digits; i++) {
        uint32_t divisor = 1;
        uint8_t j;
        uint8_t digit;

        for (j = (uint8_t)(i + 1); j < digits; j++) {
            divisor *= 10U;
        }
        digit = (uint8_t)((value / divisor) % 10U);

        if (digit != 0U || i == (uint8_t)(digits - 1)) {
            started = 1;
        }
        OLED_ShowChar((uint8_t)(x + i * 6), y, started ? (char)('0' + digit) : ' ', color);
    }
}

void OLED_ShowSigned(uint8_t x, uint8_t y, int32_t value, uint8_t color)
{
    uint32_t magnitude;
    char digits[10];
    uint8_t count = 0;

    if (value < 0) {
        OLED_ShowChar(x, y, '-', color);
        x = (uint8_t)(x + 6);
        /* INT32_MIN cannot be negated directly in int32_t. */
        magnitude = (uint32_t)(-(value + 1)) + 1U;
    } else {
        magnitude = (uint32_t)value;
    }

    do {
        digits[count++] = (char)('0' + (magnitude % 10U));
        magnitude /= 10U;
    } while (magnitude != 0U && count < sizeof(digits));

    while (count > 0U) {
        OLED_ShowChar(x, y, digits[--count], color);
        x = (uint8_t)(x + 6);
    }
}

void OLED_ShowChinese11(uint8_t x, uint8_t y, uint8_t glyph_id, uint8_t color)
{
    const uint8_t *glyph;
    uint8_t row;
    uint8_t col;
    uint8_t background = color ? 0U : 1U;

    if (glyph_id >= CN_COUNT) {
        return;
    }
    glyph = OLED_Font11x11[glyph_id];

    for (row = 0; row < 11; row++) {
        uint16_t bits = (uint16_t)glyph[row * 2] | ((uint16_t)glyph[row * 2 + 1] << 8);
        for (col = 0; col < 11; col++) {
            OLED_DrawPixel((uint8_t)(x + col), (uint8_t)(y + row),
                           (bits & (uint16_t)(1U << col)) ? color : background);
        }
    }
}

void OLED_ShowChinese11String(uint8_t x, uint8_t y, const uint8_t *glyph_ids, uint8_t color)
{
    while (*glyph_ids != CN_END && x <= (OLED_WIDTH - 11)) {
        OLED_ShowChinese11(x, y, *glyph_ids, color);
        x = (uint8_t)(x + 11);
        glyph_ids++;
    }
}

#ifdef MENU_TEST
void OLED_TestCopyGram(uint8_t *destination)
{
    memcpy(destination, s_gram, sizeof(s_gram));
}
#endif
