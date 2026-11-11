#ifndef OLED_FONT_H
#define OLED_FONT_H

#include <stdint.h>

/* Number of Chinese glyphs in OLED_Font12x12. */
typedef enum {
    CN_ZHU,
    CN_CAI,
    CN_DAN,
    CN_KONG,
    CN_ZHI,
    CN_XIN,
    CN_XI,
    CN_XIAN_SHI,
    CN_SHI,
    CN_XUN,
    CN_XIAN,
    CN_GONG,
    CN_NENG,
    CN_TUO,
    CN_ZHAN,
    CN_COUNT
} OLED_ChineseId;

#define CN_END 0xFF

extern const uint8_t OLED_Font6x8[95][6];
extern const uint8_t OLED_Font11x11[CN_COUNT][22];

#endif
