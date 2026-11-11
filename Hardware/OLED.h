#ifndef OLED_H
#define OLED_H

#include <stdint.h>

#define OLED_WIDTH   128
#define OLED_HEIGHT  64

void OLED_Init(void);
void OLED_Clear(void);
void OLED_Refresh(void);

void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t color);
void OLED_FillRect(uint8_t x, uint8_t y, uint8_t width, uint8_t height, uint8_t color);
void OLED_DrawHLine(uint8_t x, uint8_t y, uint8_t width, uint8_t color);
void OLED_DrawArrow(uint8_t x, uint8_t y, uint8_t color);

void OLED_ShowChar(uint8_t x, uint8_t y, char ch, uint8_t color);
void OLED_ShowString(uint8_t x, uint8_t y, const char *text, uint8_t color);
void OLED_ShowUnsigned(uint8_t x, uint8_t y, uint32_t value, uint8_t digits, uint8_t color);
void OLED_ShowSigned(uint8_t x, uint8_t y, int32_t value, uint8_t color);
void OLED_ShowChinese11(uint8_t x, uint8_t y, uint8_t glyph_id, uint8_t color);
void OLED_ShowChinese11String(uint8_t x, uint8_t y, const uint8_t *glyph_ids, uint8_t color);

#ifdef MENU_TEST
void OLED_TestCopyGram(uint8_t *destination);
#endif

#endif
