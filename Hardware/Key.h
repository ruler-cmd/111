#ifndef KEY_H
#define KEY_H

#include <stdint.h>

typedef enum {
    KEY_NONE = 0,
    KEY1 = 1,
    KEY2 = 2,
    KEY3 = 3,
    KEY4 = 4
} KeyCode;

void Key_Init(void);
KeyCode Key_Scan(void);

#endif
