/*
 * ssd1306.c
 * 
 * Created on:  2018-10-3
 *     Auther:  ChenFY  
 * 
 */
#ifndef __SSD1306_H__
#define __SSD1306_H__

#include "stdint.h"
#include "driver/spi_master.h"
#include "hspi.h"
#include "display.h"

#define SSD1306_WIDTH   128
#define SSD1306_HEIGHT  64

extern void ssd1306_write_cmd(const uint8_t cmd);
extern void ssd1306_write_data(const uint8_t data);
extern void ssd1306_setpin_dc_callback(spi_transaction_t *t);
extern void ssd1306_clear(void);
extern void ssd1306_set_pos(uint8_t x, uint8_t y);
extern void ssd1306_display_char(uint8_t x, uint8_t y, char chr);
extern void ssd1306_display_string(uint8_t x, uint8_t y, char *str);
extern void ssd1306_init(void);

#endif