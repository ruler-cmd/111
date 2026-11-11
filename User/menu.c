#include "menu.h"
#include "OLED.h"
#include "OLED_Font.h"
#include "LED.h"
#include "UART.h"
#include "BoardConfig.h"

#include <string.h>

/* Chinese glyph sequences.  Storing IDs instead of string literals keeps the
 * source encoding independent (Keil can then compile it as ASCII/UTF-8). */
static const uint8_t s_title[] = {CN_ZHU, CN_CAI, CN_DAN, CN_END};
static const uint8_t s_led_cn[] = {CN_KONG, CN_ZHI, CN_END};
static const uint8_t s_info_cn[] = {CN_XIN, CN_XI, CN_XIAN_SHI, CN_SHI, CN_END};
static const uint8_t s_line_cn[] = {CN_XUN, CN_XIAN, CN_GONG, CN_NENG, CN_END};
static const uint8_t s_extra_cn[] = {CN_TUO, CN_ZHAN, CN_GONG, CN_NENG, CN_END};

typedef struct {
    const char *prefix;       /* ASCII part, e.g. "LED " */
    const uint8_t *chinese;   /* glyph IDs, terminated by CN_END */
    MenuPage page;
} MenuItem;

static const MenuItem s_items[] = {
    {"LED ", s_led_cn, PAGE_LED},
    {"", s_info_cn, PAGE_INFO},
    {"", s_line_cn, PAGE_LINE},
    {"", s_extra_cn, PAGE_EXTRA},
};

#define MENU_ITEM_COUNT ((uint8_t)(sizeof(s_items) / sizeof(s_items[0])))

static MenuPage s_page;
static uint8_t s_cursor;
static uint8_t s_dirty;
static uint32_t s_uptime_ms;
static uint16_t s_extra_count;

static void Menu_DrawMain(void)
{
    uint8_t i;

    OLED_ShowChinese11String(4, 0, s_title, 1);
    OLED_ShowString(98, 2, "MENU", 1);
    OLED_DrawHLine(0, 12, OLED_WIDTH, 1);

    for (i = 0; i < MENU_ITEM_COUNT; i++) {
        uint8_t y = (uint8_t)(13 + i * 13);
        uint8_t selected = (i == s_cursor) ? 1U : 0U;
        uint8_t color = selected ? 0U : 1U;
        uint8_t x = 14;
        const MenuItem *item = &s_items[i];

        if (selected) {
            OLED_FillRect(0, y, OLED_WIDTH, 12, 1);
            OLED_DrawArrow(3, (uint8_t)(y + 2), color);
        }

        OLED_ShowString(x, (uint8_t)(y + 2), item->prefix, color);
        x = (uint8_t)(x + strlen(item->prefix) * 6U);
        OLED_ShowChinese11String(x, y, item->chinese, color);
    }
}

static void Menu_DrawTitle(const char *ascii, const uint8_t *chinese)
{
    uint8_t x = 2;

    OLED_ShowString(x, 2, ascii, 1);
    x = (uint8_t)(x + strlen(ascii) * 6U);
    OLED_ShowChinese11String(x, 0, chinese, 1);
    OLED_DrawHLine(0, 12, OLED_WIDTH, 1);
}

static void Menu_DrawLed(void)
{
    const char *mode;

    switch (LED_GetMode()) {
        case LED_MODE_ON:
            mode = "ON";
            break;
        case LED_MODE_BLINK:
            mode = "BLINK";
            break;
        default:
            mode = "OFF";
            break;
    }

    Menu_DrawTitle("LED ", s_led_cn);
    OLED_ShowString(2, 18, "MODE:", 1);
    OLED_ShowString(38, 18, mode, 1);
    OLED_ShowString(2, 27, "LED1:", 1);
    OLED_ShowString(38, 27, LED_IsOn(0) ? "ON" : "OFF", 1);
    OLED_ShowString(2, 36, "LED2:", 1);
    OLED_ShowString(38, 36, LED_IsOn(1) ? "ON" : "OFF", 1);
    OLED_ShowString(2, 45, "KEY1:ALL KEY2:BLINK", 1);
    OLED_ShowString(2, 54, "KEY4: BACK", 1);
}

static void Menu_DrawInfo(void)
{
    Menu_DrawTitle("", s_info_cn);
    OLED_ShowString(2, 18, "NAME:", 1);
    OLED_ShowString(38, 18, INFO_NAME, 1);
    OLED_ShowString(2, 28, "ID:", 1);
    OLED_ShowString(26, 28, INFO_STUDENT_ID, 1);
    OLED_ShowString(2, 38, "A =", 1);
    OLED_ShowSigned(26, 38, UART_GetVarA(), 1);
    OLED_ShowString(2, 48, "KEY4: BACK", 1);
}

static void Menu_DrawLine(void)
{
    Menu_DrawTitle("", s_line_cn);
    OLED_ShowString(2, 18, "LINE FOLLOWER", 1);
    OLED_ShowString(2, 28, "SENSOR: N/A", 1);
    OLED_ShowString(2, 38, "STATUS: IDLE", 1);
    OLED_ShowString(2, 48, "KEY4: BACK", 1);
}

static void Menu_DrawExtra(void)
{
    Menu_DrawTitle("", s_extra_cn);
    OLED_ShowString(2, 18, "EXTRA FEATURES", 1);
    OLED_ShowString(2, 28, "COUNT:", 1);
    OLED_ShowUnsigned(44, 28, s_extra_count, 3, 1);
    OLED_ShowString(2, 38, "KEY3: +1", 1);
    OLED_ShowString(2, 48, "KEY4: BACK", 1);
}

void Menu_Init(void)
{
    s_page = PAGE_MAIN;
    s_cursor = 0;                 /* power-on arrow is on the first row */
    s_dirty = 1;
    s_uptime_ms = 0;
    s_extra_count = 0;
}

void Menu_HandleKey(KeyCode key)
{
    if (key == KEY_NONE) {
        return;
    }

    if (s_page == PAGE_MAIN) {
        if (key == KEY1) {
            s_cursor = (uint8_t)((s_cursor + MENU_ITEM_COUNT - 1U) % MENU_ITEM_COUNT);
            s_dirty = 1;
        } else if (key == KEY2) {
            s_cursor = (uint8_t)((s_cursor + 1U) % MENU_ITEM_COUNT);
            s_dirty = 1;
        } else if (key == KEY3) {
            s_page = s_items[s_cursor].page;
            s_dirty = 1;
        }
        return;
    }

    /* Every sub-page uses KEY4 as "back"; returning also turns both LEDs off. */
    if (key == KEY4) {
        LED_AllOff();
        s_page = PAGE_MAIN;
        s_dirty = 1;
        return;
    }

    if (s_page == PAGE_LED && key == KEY1) {
        LED_ToggleAll();
        s_dirty = 1;
    } else if (s_page == PAGE_LED && key == KEY2) {
        LED_StartBlink(s_uptime_ms);
        s_dirty = 1;
    } else if (s_page == PAGE_EXTRA && key == KEY3) {
        s_extra_count++;
        s_dirty = 1;
    }
}

void Menu_Tick(void)
{
    s_uptime_ms += 10U;

    /* Blink timing and OLED update are both driven from this 10 ms tick. */
    if (LED_Task(s_uptime_ms)) {
        s_dirty = 1;
    }

    /* A UART write changes variable a; refresh the info page immediately. */
    if (UART_VarChanged()) {
        s_dirty = 1;
    }
}

void Menu_Refresh(void)
{
    if (!s_dirty) {
        return;
    }

    OLED_Clear();
    switch (s_page) {
        case PAGE_MAIN:
            Menu_DrawMain();
            break;
        case PAGE_LED:
            Menu_DrawLed();
            break;
        case PAGE_INFO:
            Menu_DrawInfo();
            break;
        case PAGE_LINE:
            Menu_DrawLine();
            break;
        case PAGE_EXTRA:
            Menu_DrawExtra();
            break;
        default:
            s_page = PAGE_MAIN;
            Menu_DrawMain();
            break;
    }
    OLED_Refresh();
    s_dirty = 0;
}

#ifdef MENU_TEST
MenuPage Menu_TestGetPage(void)
{
    return s_page;
}

uint8_t Menu_TestGetCursor(void)
{
    return s_cursor;
}

uint16_t Menu_TestGetExtraCount(void)
{
    return s_extra_count;
}
#endif
