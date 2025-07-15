#ifndef __LCD_FONT_H
#define __LCD_FONT_H 	   

#include <stdint.h>

typedef enum {
    FONT_1206 = 0,
    FONT_1608,
    FONT_2412,
    FONT_3216,
    FONT_MAX        = FONT_3216,
    FONT_DEFAULT    = FONT_1608,
} font_type;

typedef struct __lcd_font {
    uint16_t width;
    uint16_t height;
    uint16_t bytes;
    uint16_t front_color;
    uint16_t back_color;
    font_type type;
    const uint8_t* addr;
} lcd_font;

extern const lcd_font lcd_fonts[];

extern const unsigned char ascii_1206[][12];
extern const unsigned char ascii_1608[][16];
extern const unsigned char ascii_2412[][48];
extern const unsigned char ascii_3216[][64];

#endif


