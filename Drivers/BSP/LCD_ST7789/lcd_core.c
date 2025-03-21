#include <stdio.h>
#include <stdarg.h>
 
#include "lcd.h"

lcd_hw lcd_hw_0_96 = {
    .name   = "0.96 inch",
    .type   = LCD_0_96_INCH,
    .rotate = LCD_ROTATE_270,
    .width  = 160,
    .height = 80,
};

lcd_hw lcd_hw_1_14 = {
    .name   = "1.14 inch",
    .type   = LCD_1_14_INCH,
    .rotate = LCD_ROTATE_270,
    .width  = 240,
    .height = 135,
};

lcd_hw lcd_hw_1_47 = {
    .name   = "1.47 inch",
    .type   = LCD_1_47_INCH,
    .rotate = LCD_ROTATE_270,
    .width  = 320,
    .height = 172,
};

lcd_hw lcd_hw_2_00 = {
    .name   = "2.00 inch",
    .type   = LCD_2_00_INCH,
    .rotate = LCD_ROTATE_270,
    .width  = 320,
    .height = 240,
};

lcd_hw* lcd_hw_desc[] = {
    &lcd_hw_0_96,
    &lcd_hw_1_14,
    &lcd_hw_1_47,
    &lcd_hw_2_00,
};

uint8_t lcd_cfg_order[][4] = {
    /* 0.96 inch */
    {0x08, 0xC8, 0x78, 0xA8},
    /* 1.14 inch */
    {0x00, 0xC0, 0x70, 0xA0},
    /* 1.47 inch */
    {0x00, 0xC0, 0x70, 0xA0},
    /* 2.00 inch */
    {0x00, 0xC0, 0x70, 0xA0},    
};

uint8_t lcd_cfg_address[][4][4] = {
    /* 0.96 inch */
    { {26, 26,  1,  1}, {26, 26,  1,  1}, { 1,  1, 26, 26}, { 1,  1, 26, 26} },
    /* 1.14 inch */
    { {52, 52, 40, 40}, {53, 53, 40, 40}, {40, 40, 53, 53}, {40, 40, 52, 52} },
    /* 1.47 inch */
    { {34, 34,  0,  0}, {34, 34,  0,  0}, { 0,  0, 34, 34}, { 0,  0, 34, 34} },
    /* 2.00 inch */
    { { 0,  0,  0,  0}, { 0,  0,  0,  0}, { 0,  0,  0,  0}, { 0,  0,  0,  0} },    
};

/******************************************************************************
      函数说明：设置起始和结束地址
      入口数据：x1,x2 设置列的起始和结束地址
                y1,y2 设置行的起始和结束地址
      返回值：  无
******************************************************************************/
void lcd_set_address(lcd* plcd, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    /* 列地址设置 */
    lcd_config_reg(plcd->io, 0x2a);
    lcd_write_halfword(plcd->io, x1 + lcd_cfg_address[plcd->hw->type][plcd->hw->rotate][0]);
    lcd_write_halfword(plcd->io, x2 + lcd_cfg_address[plcd->hw->type][plcd->hw->rotate][1]);
    /* 行地址设置 */
    lcd_config_reg(plcd->io, 0x2b);
    lcd_write_halfword(plcd->io, y1 + lcd_cfg_address[plcd->hw->type][plcd->hw->rotate][2]);
    lcd_write_halfword(plcd->io, y2 + lcd_cfg_address[plcd->hw->type][plcd->hw->rotate][3]);
    /* 储存器写 */
    lcd_config_reg(plcd->io, 0x2c);
}

void lcd_config_reg_1_14(lcd* plcd)
{
    /* sleep out */
    lcd_config_reg(plcd->io, 0x11);
	lcd_delay(5);

    /* 设置显示方向 */
    lcd_config_reg(plcd->io, 0x36, lcd_cfg_order[plcd->hw->type][plcd->hw->rotate]);

    /* 16bit/pixel */
    lcd_config_reg(plcd->io, 0x3A, 0x05);

    /* Porch Setting */
    lcd_config_reg(plcd->io, 0xB2, 0x0C, 0x0C, 0x00, 0x33, 0x33);

    /*  Gate Control */
    lcd_config_reg(plcd->io, 0xB7, 0x35);

    /* VCOM Setting */   
    lcd_config_reg(plcd->io, 0xBB, 0x19);

    lcd_config_reg(plcd->io, 0xC0, 0x2C);
    
    /* VDV and VRH Command Enable */
    lcd_config_reg(plcd->io, 0xC2, 0x01);

    /* VRH Set */ 
    lcd_config_reg(plcd->io, 0xC3, 0x12);

    /* VDV Set */
    lcd_config_reg(plcd->io, 0xC4, 0x20);

    /* Frame Rate Control in Normal Mode */
    lcd_config_reg(plcd->io, 0xC6, 0x0F);

    /* Power Control 1 */
    lcd_config_reg(plcd->io, 0xD0, 0xA4, 0xA1);

    /* Positive Voltage Gamma Control */
    lcd_config_reg(plcd->io, 0xE0, 0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23);
    
    /* Negative Voltage Gamma Control */
    lcd_config_reg(plcd->io, 0xE1, 0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23);

    /* Display inversion */
    lcd_config_reg(plcd->io, 0x21);
    
    /* Display on */
    lcd_config_reg(plcd->io, 0x29);
}

void lcd_config_reg_1_47(lcd* plcd)
{
    /* sleep out */
    lcd_config_reg(plcd->io, 0x11);
	lcd_delay(5);

    /* 设置显示方向 */
    lcd_config_reg(plcd->io, 0x36, lcd_cfg_order[plcd->hw->type][plcd->hw->rotate]);

    /* 16bit/pixel */
    lcd_config_reg(plcd->io, 0x3A, 0x05);

    /* Porch Setting */
    lcd_config_reg(plcd->io, 0xB2, 0x0C, 0x0C, 0x00, 0x33, 0x33);

    /*  Gate Control */
    lcd_config_reg(plcd->io, 0xB7, 0x35);

    /* VCOM Setting */   
    lcd_config_reg(plcd->io, 0xBB, 0x35);

    lcd_config_reg(plcd->io, 0xC0, 0x2C);
    
    /* VDV and VRH Command Enable */
    lcd_config_reg(plcd->io, 0xC2, 0x01);

    /* VRH Set */ 
    lcd_config_reg(plcd->io, 0xC3, 0x13);

    /* VDV Set */
    lcd_config_reg(plcd->io, 0xC4, 0x20);

    /* Frame Rate Control in Normal Mode */
    lcd_config_reg(plcd->io, 0xC6, 0x0F);

    /* Power Control 1 */
    lcd_config_reg(plcd->io, 0xD0, 0xA4, 0xA1);
    
    lcd_config_reg(plcd->io, 0xD6, 0xA1);

    /* Positive Voltage Gamma Control */
    lcd_config_reg(plcd->io, 0xE0, 0xF0, 0x00, 0x04, 0x04, 0x04, 0x05, 0x29, 0x33, 0x3E, 0x38, 0x12, 0x12, 0x28, 0x30);
    
    /* Negative Voltage Gamma Control */
    lcd_config_reg(plcd->io, 0xE1, 0xF0, 0x07, 0x0A, 0x0D, 0x0B, 0x07, 0x28, 0x33, 0x3E, 0x36, 0x14, 0x14, 0x29, 0x32);

    /* Display inversion */
    lcd_config_reg(plcd->io, 0x21);
    
    lcd_config_reg(plcd->io, 0x11);
    
    lcd_delay(120);
    
    /* Display on */
    lcd_config_reg(plcd->io, 0x29);
}

void lcd_config_reg_2_00(lcd* plcd)
{
    /* sleep out */
    lcd_config_reg(plcd->io, 0x11);
	lcd_delay(5);

    /* 设置显示方向 */
    lcd_config_reg(plcd->io, 0x36, lcd_cfg_order[plcd->hw->type][plcd->hw->rotate]);

    /* 16bit/pixel */
    lcd_config_reg(plcd->io, 0x3A, 0x05);

    /* Porch Setting */
    lcd_config_reg(plcd->io, 0xB2, 0x0C, 0x0C, 0x00, 0x33, 0x33);

    /*  Gate Control */
    lcd_config_reg(plcd->io, 0xB7, 0x35);

    /* VCOM Setting */   
    lcd_config_reg(plcd->io, 0xBB, 0x32);

    /* VDV and VRH Command Enable */
    lcd_config_reg(plcd->io, 0xC2, 0x01);

    /* VRH Set */ 
    lcd_config_reg(plcd->io, 0xC3, 0x15);

    /* VDV Set */
    lcd_config_reg(plcd->io, 0xC4, 0x20);

    /* Frame Rate Control in Normal Mode */
    lcd_config_reg(plcd->io, 0xC6, 0x0F);

    /* Power Control 1 */
    lcd_config_reg(plcd->io, 0xD0, 0xA4, 0xA1);

    /* Positive Voltage Gamma Control */
    lcd_config_reg(plcd->io, 0xE0, 0xD0, 0x08, 0x0E, 0x09, 0x09, 0x05, 0x31, 0x33, 0x48, 0x17, 0x14, 0x15, 0x31, 0x34);
    
    /* Negative Voltage Gamma Control */
    lcd_config_reg(plcd->io, 0xE1, 0xD0, 0x08, 0x0E, 0x09, 0x09, 0x15, 0x31, 0x33, 0x48, 0x17, 0x14, 0x15, 0x31, 0x34);

    /* Display inversion */
    lcd_config_reg(plcd->io, 0x21);
    
    /* Display on */
    lcd_config_reg(plcd->io, 0x29);
}

void lcd_init_hw(lcd* plcd)
{
    lcd_io_rst(plcd->io, 0);
	lcd_delay(100);
    lcd_io_rst(plcd->io, 1);
	lcd_delay(100);

	/* 打开背光 */
    lcd_io_bl(plcd->io, 1);
    lcd_delay(100);

    switch(plcd->hw->type) {
        case LCD_0_96_INCH:
            break;
        case LCD_1_14_INCH: 
            lcd_config_reg_1_14(plcd);
            break;
        case LCD_1_47_INCH:
            lcd_config_reg_1_47(plcd);
            break;
        case LCD_2_00_INCH:
            lcd_config_reg_2_00(plcd);
            break;
    }
}

void lcd_write_reg_data(lcd_io* lcdio, int len, ...)
{
    va_list args;
    va_start(args, len);
    
    lcd_write_reg(lcdio, (uint8_t)va_arg(args, unsigned int));
    
    for (int i = 1; i < len; i++) {
        lcd_write_byte(lcdio, (uint8_t)va_arg(args, unsigned int));
    }

	va_end(args);     
}

void lcd_init_dev(lcd* plcd, lcd_type type, lcd_rotate rotate)
{
    uint16_t width;
    uint16_t height;
    
    plcd->hw = lcd_hw_desc[type];
    
    switch (rotate) {
	case LCD_ROTATE_0:
	case LCD_ROTATE_180:
		width =  plcd->hw->height;
		height = plcd->hw->width;
		break;
	default:
		width =  plcd->hw->width;
		height = plcd->hw->height;
	}
    
    plcd->hw->width  = width;
    plcd->hw->height = height;
    plcd->hw->rotate = rotate;
    lcd_set_font(plcd, FONT_DEFAULT, GBLUE, BLACK);

    lcd_init_hw(plcd);
    lcd_clear(plcd, BLACK);
}

void lcd_clear(lcd* plcd, uint16_t color)
{
    lcd_fill(plcd, 0, 0, plcd->hw->width - 1, plcd->hw->height - 1, color);
}

void lcd_draw_point(lcd* plcd, uint16_t x, uint16_t y, uint16_t color)
{
    lcd_set_address(plcd, x, y, x, y);
    lcd_write_halfword(plcd->io, color);
}

void lcd_fill(lcd* plcd, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    uint16_t width, height;
    width  = x2 - x1 + 1;
    height = y2 - y1 + 1;
    
    color = (color << 8) | (color >> 8);

    if(!plcd->line_buffer) {
        for(int i = 0; i < height; i++) {
            for(int j = 0; j < width; j++) {
                lcd_draw_point(plcd, x1 + j, y1 + i, color);
            }
        }
    } else {
        for(int i = 0; i < width; i++)
            plcd->line_buffer[i] = color;

        lcd_set_address(plcd, x1, y1, x2, y2);
        for(int i = 0; i < height; i++)
            lcd_write_bulk(plcd->io, (uint8_t *)&plcd->line_buffer[0], width * 2);
    }
}

/* x1, y1:起点坐标 x2, y2:终点坐标*/
void lcd_draw_line(lcd* plcd, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    int xerr = 0, yerr = 0;
    int delta_x, delta_y, distance; 
    int incx, incy, pos_x, pos_y; 

    delta_x = x2 - x1;
    delta_y = y2 - y1; 
    pos_x = x1;
    pos_y = y1; 

    if(delta_x > 0)
        incx = 1;
    else if(delta_x == 0)
        incx = 0;
    else {
        incx = -1;
        delta_x = -delta_x;
    }

    if(delta_y > 0)
        incy = 1; 
    else if(delta_y == 0)
        incy = 0;
    else {
        incy = -1;
        delta_y = -delta_y;
    } 

    distance = delta_x > delta_y ? delta_x : delta_y;

    for(int i = 0; i <= distance + 1; i++ ) {
        lcd_draw_point(plcd, pos_x, pos_y, color);
        xerr += delta_x; 
        yerr += delta_y; 
        if(xerr > distance) { 
            xerr -= distance; 
            pos_x += incx; 
        } 
        if(yerr > distance) { 
            yerr -= distance; 
            pos_y += incy; 
        }
    }
}

void lcd_draw_rectangle(lcd* plcd, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    lcd_draw_line(plcd, x1, y1, x2, y1, color);
    lcd_draw_line(plcd, x1, y1, x1, y2, color);
    lcd_draw_line(plcd, x1, y2, x2, y2, color);
    lcd_draw_line(plcd, x2, y1, x2, y2, color);
}

void lcd_set_font(lcd* plcd, font_type type, uint16_t front_color, uint16_t back_color)
{
    plcd->font = lcd_fonts[type];
    plcd->font.front_color = front_color;
    plcd->font.back_color  = back_color;
}

/* 在指定位置显示一个字符 */
void lcd_show_char(lcd* plcd, uint16_t x, uint16_t y, uint16_t chr)
{
    uint8_t width = 0;
    chr = chr - ' ';
    
    if(x > plcd->hw->width - plcd->font.width ||
       y > plcd->hw->height - plcd->font.height) {
        return;
    }

    lcd_set_address(plcd, x, y, x + plcd->font.width - 1, y + plcd->font.height - 1);
    
    for(int idx = 0; idx < plcd->font.bytes; idx++) {
        uint8_t data = plcd->font.addr[chr * plcd->font.bytes + idx];
        for(int pixel = 0; pixel < 8; pixel++) {
            if(data & 0x01)
                lcd_write_halfword(plcd->io, plcd->font.front_color);
            else
                lcd_write_halfword(plcd->io, plcd->font.back_color);
            data >>= 1;

            width++;
            if(width == plcd->font.width) {
                width = 0;
                break;
            }
        }
    }  	 	  
}

/* *p:字符串起始地址 */
void lcd_show_string(lcd* plcd, uint16_t x, uint16_t y, const uint8_t *p)
{
    while(*p != '\0') {
        if(x > plcd->hw->width - plcd->font.width) {
            x = 0;
            y += plcd->font.height;
        }

        lcd_show_char(plcd, x, y, *p++); 
        x += plcd->font.width;
    }
}

void lcd_print(lcd* plcd, uint16_t x, uint16_t y, const char *fmt, ...)
{
    unsigned char buffer[128] = { 0 }; 
    va_list ap;
    
    va_start(ap,fmt);
    vsnprintf((char*)buffer, sizeof(buffer), fmt, ap);
    va_end(ap);

    lcd_show_string(plcd, x, y, buffer);
}

/******************************************************************************
      函数说明：显示图片
      入口数据：x,y起点坐标
                length 图片长度
                width  图片宽度
                pic[]  图片数组    
      返回值：  无
******************************************************************************/
void lcd_show_picture(lcd* plcd, uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t* pic)
{
    lcd_set_address(plcd, x, y, x + width - 1, y + height - 1);
    lcd_write_bulk(plcd->io, pic, width * height * 2);
}
