#ifndef __LCD_H
#define __LCD_H

#include <stdint.h>
#include "lcd_port.h"
#include "lcd_font.h"

typedef enum {
    LCD_0_96_INCH = 0,
    LCD_1_14_INCH,
    LCD_1_47_INCH,
    LCD_2_00_INCH,
} lcd_type;

typedef enum {
    LCD_ROTATE_0 = 0,
    LCD_ROTATE_180,
    LCD_ROTATE_90,
    LCD_ROTATE_270
} lcd_rotate;

typedef struct __lcd_hw {
    char* name;
    lcd_type type;
    lcd_rotate rotate;
    
    uint16_t width;
    uint16_t height;
} lcd_hw;

typedef struct __lcd {
    lcd_io* io;
    lcd_hw* hw;
    lcd_font font;
    
    void (*init)(lcd_io*, lcd_hw*);
    void (*print)(lcd_io*, uint16_t x, uint16_t y, uint16_t colar, const char *fmt, ...);

    uint16_t* line_buffer;
    uint16_t* frame_buffer; // frame_w/h/size
    uint32_t timeout;
} lcd;

extern lcd_hw lcd_hw_0_96;
extern lcd_hw lcd_hw_1_14;
extern lcd_hw lcd_hw_1_47;
extern lcd_hw lcd_hw_2_00;

void lcd_init_dev(lcd* plcd, lcd_type type, lcd_rotate rotate);
void lcd_init_hw(lcd* plcd);
void lcd_clear(lcd* plcd, uint16_t color);

void lcd_draw_point(lcd* plcd, uint16_t x, uint16_t y, uint16_t color);
void lcd_show_char(lcd* plcd, uint16_t x, uint16_t y, uint16_t chr);
void lcd_show_string(lcd* plcd, uint16_t x, uint16_t y, const uint8_t *p);
void lcd_print(lcd* plcd, uint16_t x, uint16_t y, const char *fmt, ...);
void lcd_draw_line(lcd* plcd, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void lcd_draw_rectangle(lcd* plcd, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void lcd_fill(lcd* plcd, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);
void lcd_set_font(lcd* plcd, font_type type, uint16_t front_color, uint16_t back_color);
void lcd_show_picture(lcd* plcd, uint16_t x, uint16_t y, uint16_t length, uint16_t width, uint8_t* pic);
void lcd_set_address(lcd* plcd, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

void lcd_write_reg_data(lcd_io* lcdio, int len, ...);
#define NUMARGS(...)  (sizeof((int[]){__VA_ARGS__}) / sizeof(int))
#define lcd_config_reg(x, ...)   lcd_write_reg_data(x, NUMARGS(__VA_ARGS__), __VA_ARGS__)

//画笔颜色
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE           	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //棕色
#define BRRED 			 0XFC07 //棕红色
#define GRAY  			 0X8430 //灰色
#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色  
#define GRAYBLUE       	 0X5458 //灰蓝色
#define LIGHTGREEN     	 0X841F //浅绿色
#define LGRAY 			 0XC618 //浅灰色(PANNEL),窗体背景色
#define LGRAYBLUE        0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE           0X2B12 //浅棕蓝色(选择条目的反色)

#endif




