/*
 * ssd1351.c
 *
 *  Created on: 2018-03-14 18:04
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>

#include "spi.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "ssd1351.h"
#include "fonts.h"

#define abs(x) ((x)>0?(x):-(x))
#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))

#define SSD1351_GPIO_PIN_DC   23
#define SSD1351_GPIO_PIN_RST  14

#define write_reg(g, reg, data)		{ write_cmd(g, reg); write_data(g, data); }
#define init_board(g)           ssd1351_init_board()
#define setpin_reset(g, rst)    ssd1351_setpin_reset(rst)
#define write_cmd(g, cmd)       ssd1351_write_cmd(cmd)
#define write_data(g, data)     ssd1351_write_data(data)
#define refresh_gram(g, gram)   ssd1351_refresh_gram(gram)

spi_transaction_t hspi_trans[6];

static const uint8_t gray_scale_table[] = {
    0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11,
    0x12, 0x13, 0x15, 0x17, 0x19, 0x1B, 0x1D, 0x1F,
    0x21, 0x23, 0x25, 0x27, 0x2A, 0x2D, 0x30, 0x33,
    0x36, 0x39, 0x3C, 0x3F, 0x42, 0x45, 0x48, 0x4C,
    0x50, 0x54, 0x58, 0x5C, 0x60, 0x64, 0x68, 0x6C,
    0x70, 0x74, 0x78, 0x7D, 0x82, 0x87, 0x8C, 0x91,
    0x96, 0x9B, 0xA0, 0xA5, 0xAA, 0xAF, 0xB4
};

void ssd1351_init_board(void)
{
    hspi_init();
    gpio_set_direction(SSD1351_GPIO_PIN_DC,  GPIO_MODE_OUTPUT);
    gpio_set_direction(SSD1351_GPIO_PIN_RST, GPIO_MODE_OUTPUT);
    gpio_set_level(SSD1351_GPIO_PIN_DC,  0);
    gpio_set_level(SSD1351_GPIO_PIN_RST, 0);



    memset(hspi_trans, 0, sizeof(hspi_trans));
}

void ssd1351_setpin_dc(spi_transaction_t *t)
{
    int dc = (int)t->user;
    gpio_set_level(SSD1351_GPIO_PIN_DC, dc);
}

void ssd1351_setpin_reset(uint8_t rst)
{
    gpio_set_level(SSD1351_GPIO_PIN_RST, rst);
}

void ssd1351_write_cmd(uint8_t cmd)
{
    esp_err_t ret;

    hspi_trans[0].length = 8;
    hspi_trans[0].tx_buffer = &cmd;
    hspi_trans[0].user = (void*)0;

    ret = spi_device_transmit(hspi, &hspi_trans[0]);
    assert(ret == ESP_OK);
}

void ssd1351_write_data(uint8_t data)
{
    esp_err_t ret;

    hspi_trans[0].length = 8;
    hspi_trans[0].tx_buffer = &data;
    hspi_trans[0].user = (void*)1;

    ret = spi_device_transmit(hspi, &hspi_trans[0]);
    assert(ret == ESP_OK);
}

void ssd1351_draw_point(unsigned char chXpos, unsigned char chYpos, unsigned int hwColor)
{
	if (chXpos >= SSD1351_SCREEN_WIDTH  || chYpos >= SSD1351_SCREEN_HEIGHT) {
		return;
	}

    ssd1351_write_cmd(SSD1351_SET_COLUMN_ADDRESS);
    ssd1351_write_data(chXpos);
    ssd1351_write_data(chXpos);

    ssd1351_write_cmd(SSD1351_SET_ROW_ADDRESS );
    ssd1351_write_data(chXpos);
    ssd1351_write_data(chXpos);

    ssd1351_write_cmd(SSD1351_WRITE_RAM  );
    ssd1351_write_data(hwColor >> 8);
    ssd1351_write_data(hwColor);
}

void ssd1351_draw_line(unsigned char chXpos0, unsigned char chYpos0, unsigned char chXpos1, unsigned char chYpos1, unsigned int hwColor)
{
	int x = chXpos1 - chXpos0;
    int y = chYpos1 - chYpos0;
    int dx = abs(x), sx = chXpos0 < chXpos1 ? 1 : -1;
    int dy = -abs(y), sy = chYpos0 < chYpos1 ? 1 : -1;
    int err = dx + dy, e2;

	if (chXpos0 >= SSD1351_SCREEN_WIDTH  || chYpos0 >= SSD1351_SCREEN_HEIGHT || chXpos1 >= SSD1351_SCREEN_WIDTH  || chYpos1 >= SSD1351_SCREEN_HEIGHT) {
		return;
	}
    
    for (;;){
        ssd1351_draw_point(chXpos0, chYpos0 , hwColor);
        e2 = 2 * err;
        if (e2 >= dy) {     
            if (chXpos0 == chXpos1) break;
            err += dy; chXpos0 += sx;
        }
        if (e2 <= dx) {
            if (chYpos0 == chYpos1) break;
            err += dx; chYpos0 += sy;
        }
    }
}

void ssd1351_draw_h_line(unsigned char chXpos, unsigned char chYpos, unsigned char chWidth, unsigned int hwColor)
{
    unsigned int i, x1 = min(chXpos + chWidth, SSD1351_SCREEN_WIDTH - 1);

    if (chXpos >= SSD1351_SCREEN_WIDTH  || chYpos >= SSD1351_SCREEN_HEIGHT) {
        return;
    }

    ssd1351_write_cmd(SSD1351_SET_COLUMN_ADDRESS);
    ssd1351_write_data(chXpos);
    ssd1351_write_data(chXpos + x1 -1);

    ssd1351_write_cmd(SSD1351_SET_ROW_ADDRESS );
    ssd1351_write_data(chXpos);
    ssd1351_write_data(chXpos);

    ssd1351_write_cmd(SSD1351_WRITE_RAM  );
    for (i = 0; i < x1; i++) {
        ssd1351_write_data(hwColor >> 8);
        ssd1351_write_data(hwColor);
    }
}

void ssd1351_draw_v_line(unsigned char chXpos, unsigned char chYpos, unsigned char chHeight, unsigned int hwColor)
{	
	unsigned int i, y1 = min(chYpos + chHeight, SSD1351_SCREEN_HEIGHT - 1);

	if (chXpos >= SSD1351_SCREEN_WIDTH  || chYpos >= SSD1351_SCREEN_HEIGHT) {
		return;
	}
	
    ssd1351_write_cmd(SSD1351_SET_COLUMN_ADDRESS);
    ssd1351_write_data(chXpos);
    ssd1351_write_data(chXpos);

    ssd1351_write_cmd(SSD1351_SET_ROW_ADDRESS );
    ssd1351_write_data(chXpos);
    ssd1351_write_data(chYpos + y1 - 1);

    ssd1351_write_cmd(SSD1351_WRITE_RAM  );
    for (i = 0; i < y1; i++) {
        ssd1351_write_data(hwColor >> 8);
        ssd1351_write_data(hwColor);
    }
}

void ssd1351_draw_column(unsigned char chXpos, unsigned char chYpos, unsigned char chHeight, unsigned int hwColor0, unsigned int hwColor1)
{
    unsigned int i;

    if (chXpos >= SSD1351_SCREEN_WIDTH  || chYpos >= SSD1351_SCREEN_WIDTH  || chYpos+chHeight >= SSD1351_SCREEN_HEIGHT) {
        return;
    }

    ssd1351_write_cmd(SSD1351_SET_COLUMN_ADDRESS);
    ssd1351_write_data(chXpos);
    ssd1351_write_data(chXpos);

    ssd1351_write_cmd(SSD1351_SET_ROW_ADDRESS );
    ssd1351_write_data(chXpos);
	ssd1351_write_data( SSD1351_SCREEN_HEIGHT - 1);

    ssd1351_write_cmd(SSD1351_WRITE_RAM  );
    for (i = chYpos; i < chYpos+chHeight; i++) {
        ssd1351_write_data(hwColor0 >> 8);
        ssd1351_write_data(hwColor0);
    }
    for (i = chYpos+chHeight; i < SSD1351_SCREEN_HEIGHT; i++) {
        ssd1351_write_data(hwColor1 >> 8);
        ssd1351_write_data(hwColor1);
    }
}

void ssd1351_draw_rect(unsigned char chXpos, unsigned char chYpos, unsigned char chWidth, unsigned char chHeight, unsigned int hwColor)
{
	if (chXpos >= SSD1351_SCREEN_WIDTH  || chYpos >= SSD1351_SCREEN_HEIGHT) {
		return;
	}

	ssd1351_draw_h_line(chXpos, chYpos, chWidth, hwColor);
	ssd1351_draw_h_line(chXpos, chYpos + chHeight, chWidth, hwColor);
	ssd1351_draw_v_line(chXpos, chYpos, chHeight, hwColor);
	ssd1351_draw_v_line(chXpos + chWidth, chYpos, chHeight + 1, hwColor);
}

void ssd1351_draw_circle(unsigned char chXpos, unsigned char chYpos, unsigned char chRadius, unsigned int hwColor)
{
    int x = -chRadius, y = 0, err = 2 - 2 * chRadius, e2;

    if (chXpos >= SSD1351_SCREEN_WIDTH  || chYpos >= SSD1351_SCREEN_HEIGHT) {
        return;
    }

    do {
        ssd1351_draw_point(chXpos - x, chYpos + y, hwColor);
        ssd1351_draw_point(chXpos + x, chYpos + y, hwColor);
        ssd1351_draw_point(chXpos + x, chYpos - y, hwColor);
        ssd1351_draw_point(chXpos - x, chYpos - y, hwColor);
        e2 = err;
        if (e2 <= y) {
            err += ++y * 2 + 1;
            if(-x == y && e2 <= x) e2 = 0;
        }
        if(e2 > x) err += ++x * 2 + 1;
    } while(x <= 0);
}

void ssd1351_draw_mono_bitmap(unsigned char chXpos, unsigned char chYpos, const unsigned char *pchBmp, unsigned char chWidth, unsigned char chHeight, unsigned int hwForeColor, unsigned int hwBackColor)
{
    unsigned char i, j, byteWidth = (chWidth + 7) / 8;

    if (chXpos >= SSD1351_SCREEN_WIDTH  || chYpos >= SSD1351_SCREEN_HEIGHT) {
        return;
    }

    ssd1351_write_cmd(SSD1351_SET_COLUMN_ADDRESS);
    ssd1351_write_data(chXpos);
    ssd1351_write_data(chXpos + chWidth - 1);

    ssd1351_write_cmd(SSD1351_SET_ROW_ADDRESS );
    ssd1351_write_data(chXpos);
    ssd1351_write_data(chYpos + chHeight - 1);

    ssd1351_write_cmd(SSD1351_WRITE_RAM  );
    for (i = 0; i < chHeight; i++) {
        for (j = 0; j < chWidth; j++) {
            if(*(pchBmp + j * byteWidth + i / 8) & (128 >> (i & 7))) {
                ssd1351_write_data(hwForeColor >> 8);
                ssd1351_write_data(hwForeColor);
            }
            else {
                ssd1351_write_data(hwBackColor >> 8);
                ssd1351_write_data(hwBackColor);
            }
        }
    }
}

void ssd1351_draw_64k_bitmap(unsigned char chXpos, unsigned char chYpos, const unsigned char *pchBmp, unsigned char chWidth, unsigned char chHeight)
{
    unsigned char i, j;

    if (chXpos >= SSD1351_SCREEN_WIDTH  || chYpos >= SSD1351_SCREEN_HEIGHT) {
        return;
    }

    ssd1351_write_cmd(SSD1351_SET_COLUMN_ADDRESS);
    ssd1351_write_data(chXpos);
    ssd1351_write_data(chXpos + chWidth - 1);

    ssd1351_write_cmd(SSD1351_SET_ROW_ADDRESS );
    ssd1351_write_data(chXpos);
    ssd1351_write_data(chYpos + chHeight - 1);

    ssd1351_write_cmd(SSD1351_WRITE_RAM  );
    for (i = 0; i < chHeight; i++) {
        for (j = 0; j < chWidth; j++) {
            ssd1351_write_data(*pchBmp++);
            ssd1351_write_data(*pchBmp++);
        }
    }
}

void ssd1351_refresh_gram(uint8_t *gram)
{
    esp_err_t ret;

    hspi_trans[0].length = 8;
    hspi_trans[0].tx_data[0] = 0x15;    // Set Column Address
    hspi_trans[0].user = (void*)0;
    hspi_trans[0].flags = SPI_TRANS_USE_TXDATA;

    hspi_trans[1].length = 2*8;
    hspi_trans[1].tx_data[0] = 0x00;    // startx
    hspi_trans[1].tx_data[1] = SSD1351_SCREEN_WIDTH - 1;    // endx
    hspi_trans[1].user = (void*)1;
    hspi_trans[1].flags = SPI_TRANS_USE_TXDATA;

    hspi_trans[2].length = 8,
    hspi_trans[2].tx_data[0] = 0x75;    // Set Row Address
    hspi_trans[2].user = (void*)0;
    hspi_trans[2].flags = SPI_TRANS_USE_TXDATA;

    hspi_trans[3].length = 2*8,
    hspi_trans[3].tx_data[0] = 0x00;    // starty
    hspi_trans[3].tx_data[1] = SSD1351_SCREEN_HEIGHT - 1;   // endy
    hspi_trans[3].user = (void*)1;
    hspi_trans[3].flags = SPI_TRANS_USE_TXDATA;

    hspi_trans[4].length = 8,
    hspi_trans[4].tx_data[0] = 0x5c;    // Set Write RAM
    hspi_trans[4].user = (void*)0;
    hspi_trans[4].flags = SPI_TRANS_USE_TXDATA;

    hspi_trans[5].length = SSD1351_SCREEN_WIDTH*SSD1351_SCREEN_HEIGHT*2*8;
    hspi_trans[5].tx_buffer = gram;
    hspi_trans[5].user = (void*)1;

    //Queue all transactions.
    for (int x=0; x<6; x++) {
        ret=spi_device_queue_trans(hspi, &hspi_trans[x], portMAX_DELAY);
        assert(ret==ESP_OK);
    }

    for (int x=0; x<6; x++) {
        spi_transaction_t* ptr;
        ret=spi_device_get_trans_result(hspi, &ptr, portMAX_DELAY);
        assert(ret==ESP_OK);
        assert(ptr==hspi_trans+x);
    }
}
void ssd1351_fill_rect(unsigned char chXpos, unsigned char chYpos, unsigned char chWidth, unsigned char chHeight, unsigned int hwColor)
{
    unsigned int i, j;

    if (chXpos >= 128 || chYpos >= 128) {
        return;
    }

    ssd1351_write_cmd(SSD1351_SET_COLUMN_ADDRESS );
    ssd1351_write_data(chXpos);
    ssd1351_write_data(chXpos + chWidth - 1);

    ssd1351_write_cmd(SSD1351_SET_ROW_ADDRESS);
    ssd1351_write_data(chYpos);
    ssd1351_write_data(chYpos + chHeight - 1);

    ssd1351_write_cmd(SSD1351_WRITE_RAM);
    for (i = 0; i < chHeight; i++) {
        for (j = 0; j < chWidth; j++) {
            ssd1351_write_data(hwColor >> 8);
            ssd1351_write_data(hwColor);
        }
    }
}
void ssd1351_fill_gram(unsigned int hwColor)
{
    unsigned char i, j;

    ssd1351_write_cmd(SSD1351_SET_COLUMN_ADDRESS);
    ssd1351_write_data(0x00 );
    ssd1351_write_data( SSD1351_SCREEN_WIDTH  - 1);

    ssd1351_write_cmd(SSD1351_SET_ROW_ADDRESS );
    ssd1351_write_data(0x00 );
	ssd1351_write_data( SSD1351_SCREEN_HEIGHT - 1);

    ssd1351_write_cmd(SSD1351_WRITE_RAM  );
    for (i = 0; i < SSD1351_SCREEN_HEIGHT; i ++) {
        for (j = 0; j < SSD1351_SCREEN_WIDTH ; j ++) {
            ssd1351_write_data(hwColor >> 8);
            ssd1351_write_data(hwColor);
        }
    }
}

void ssd1351_clear_rect(unsigned char chXpos, unsigned char chYpos, unsigned char chWidth, unsigned char chHeight)
{
    unsigned int i, j;

    if (chXpos >= SSD1351_SCREEN_WIDTH  || chYpos >= SSD1351_SCREEN_HEIGHT) {
        return;
    }

    ssd1351_write_cmd(SSD1351_SET_COLUMN_ADDRESS);
    ssd1351_write_data(chXpos);
    ssd1351_write_data(chXpos + chWidth - 1);

    ssd1351_write_cmd(SSD1351_SET_ROW_ADDRESS );
    ssd1351_write_data(chXpos);
    ssd1351_write_data(chYpos + chHeight - 1);

    ssd1351_write_cmd(SSD1351_WRITE_RAM  );
    for (i = 0; i < chHeight; i++) {
        for (j = 0; j < chWidth; j++) {
            ssd1351_write_data(0x00 );
            ssd1351_write_data(0x00 );
        }
    }
}

void ssd1351_clear_gram(void)
{
    unsigned char i, j;

    ssd1351_write_cmd(SSD1351_SET_COLUMN_ADDRESS);
    ssd1351_write_data(0x00 );
    ssd1351_write_data( SSD1351_SCREEN_WIDTH  - 1);

    ssd1351_write_cmd(SSD1351_SET_ROW_ADDRESS );
    ssd1351_write_data(0x00 );
    ssd1351_write_data( SSD1351_SCREEN_HEIGHT - 1);

    ssd1351_write_cmd(SSD1351_WRITE_RAM  );
    for (i = 0; i < SSD1351_SCREEN_HEIGHT; i++) {
        for (j = 0; j < SSD1351_SCREEN_WIDTH ; j++) {
            ssd1351_write_data(0x00 );
            ssd1351_write_data(0x00 );
        }
    }
}

void ssd1351_display_char(unsigned char chXpos, unsigned char chYpos, unsigned char chChr, unsigned char chFontIndex, unsigned int hwForeColor, unsigned int hwBackColor)
{      	
	unsigned char i, j;
	unsigned char chTemp, chYpos0 = chYpos;

	if (chXpos >= SSD1351_SCREEN_WIDTH  || chYpos >= SSD1351_SCREEN_HEIGHT) {
		return;
	}

    for (i = 0; i < fonts_width[chFontIndex] * ((fonts_height[chFontIndex] + 7) / 8); i++) {
        switch (chFontIndex) {
            case FONT_1206:
                chTemp = c_chFont1206[chChr - ' '][i];
                break;
            case FONT_1608:
                chTemp = c_chFont1608[chChr - ' '][i];
                break;
            case FONT_1616:
                chTemp = c_chFont1616[chChr - ' '][i];
                break;
            case FONT_3216:
                chTemp = c_chFont3216[chChr - ' '][i];
                break;
            default:
                chTemp = 0x00;
                break;
        }
		
        for (j = 0; j < 8; j++) {
    		if (chTemp & 0x80) {
				ssd1351_draw_point(chXpos, chYpos, hwForeColor);
    		}
    		else {
    		    ssd1351_draw_point(chXpos, chYpos, hwBackColor);
    		}
			chTemp <<= 1;
            chYpos++;

            if ((chYpos - chYpos0) == fonts_height[chFontIndex]) {
                chYpos = chYpos0;
                chXpos++;
                break;
            }
		}
    } 
}

static unsigned long _pow(unsigned char m, unsigned char n)
{
	unsigned long result = 1;
	
	while(n --) result *= m;    
	return result;
}

void ssd1351_display_num(unsigned char chXpos, unsigned char chYpos, unsigned long chNum, unsigned char chLen, unsigned char chFontIndex, unsigned int hwForeColor, unsigned int hwBackColor)
{         	
	unsigned char i;
	unsigned char chTemp, chShow = 0;

	if (chXpos >= SSD1351_SCREEN_WIDTH  || chYpos >= SSD1351_SCREEN_HEIGHT) {
		return;
	}
	
	for(i = 0; i < chLen; i++) {
		chTemp = (chNum / _pow(10, chLen - i - 1)) % 10;
		if(chShow == 0 && i < (chLen - 1)) {
			if(chTemp == 0) {
				ssd1351_display_char(chXpos + fonts_width[chFontIndex] * i, chYpos, ' ', chFontIndex, hwForeColor, hwBackColor);
				continue;
			} else {
				chShow = 1;
			}	 
		}
	 	ssd1351_display_char(chXpos + fonts_width[chFontIndex] * i, chYpos, chTemp + '0', chFontIndex, hwForeColor, hwBackColor);
	}
} 

void ssd1351_display_string(unsigned char chXpos, unsigned char chYpos, const char *pchString, unsigned char chFontIndex, unsigned int hwForeColor, unsigned int hwBackColor)
{
	if (chXpos >= SSD1351_SCREEN_WIDTH  || chYpos >= SSD1351_SCREEN_HEIGHT) {
		return;
	}
	
    while (*pchString != '\0') {       
        if (chXpos > (SSD1351_SCREEN_WIDTH  - fonts_width[chFontIndex])) {
			chXpos = 0;
			chYpos += fonts_height[chFontIndex];
			if (chYpos > (SSD1351_SCREEN_HEIGHT - fonts_height[chFontIndex])) {
				chYpos = chXpos = 0;
				ssd1351_clear_gram();
			}
		}
		
        ssd1351_display_char(chXpos, chYpos, *pchString, chFontIndex, hwForeColor, hwBackColor);
        chXpos += fonts_width[chFontIndex];
        pchString++;
    } 
}

void ssd1351_horizontal_scrolling(unsigned char chYpos, unsigned char chHeight, unsigned char chDirection, unsigned char chInterval)
{
    if (chYpos >= SSD1351_SCREEN_WIDTH  || (chYpos+chHeight) >= SSD1351_SCREEN_WIDTH ) {
        return;
    }

    ssd1351_write_cmd(HORIZONTAL_SCROLLING_SETUP);
    ssd1351_write_data(chDirection);
    ssd1351_write_data(chYpos);
    ssd1351_write_data(chHeight);
    ssd1351_write_data(0x00 );
    ssd1351_write_data(chInterval);

    ssd1351_write_cmd(ACTIVATE_SCROLLING);
}

void ssd1351_deactivate_scrolling(void)
{
    ssd1351_write_cmd(DEACTIVATE_SCROLLING);
}

void ssd1351_show_checkerboard(void)
{
    unsigned char i,j;

    ssd1351_write_cmd(SSD1351_SET_COLUMN_ADDRESS);
    ssd1351_write_data(0x00 );
    ssd1351_write_data( SSD1351_SCREEN_WIDTH  - 1);

    ssd1351_write_cmd(SSD1351_SET_ROW_ADDRESS );
    ssd1351_write_data(0x00 );
ssd1351_write_data( SSD1351_SCREEN_HEIGHT - 1);

    ssd1351_write_cmd(SSD1351_WRITE_RAM  );
    for (i=0; i<SSD1351_SCREEN_HEIGHT/2; i++) {
        for (j=0; j<SSD1351_SCREEN_WIDTH /2; j++) {
            ssd1351_write_data(0xFF );
            ssd1351_write_data(0xFF );
            ssd1351_write_data(0x00 );
            ssd1351_write_data(0x00 );
        }
        for (j=0; j<SSD1351_SCREEN_WIDTH /2; j++) {
            ssd1351_write_data(0x00 );
            ssd1351_write_data(0x00 );
            ssd1351_write_data(0xFF );
            ssd1351_write_data(0xFF );
        }
    }
}
void ssd1351_show_rainbow(void)
{
    // White => Column 0~15
    ssd1351_fill_rect(0x00, 0x00, 0x10, SSD1351_SCREEN_HEIGHT, White);

    // Yellow => Column 16~31
    ssd1351_fill_rect(0x10, 0x00, 0x10, SSD1351_SCREEN_HEIGHT, Yellow);

    // Purple => Column 32~47
    ssd1351_fill_rect(0x20, 0x00, 0x10, SSD1351_SCREEN_HEIGHT, Magenta);

    // Cyan => Column 48~63
    ssd1351_fill_rect(0x30, 0x00, 0x10, SSD1351_SCREEN_HEIGHT, Cyan);

    // Red => Column 64~79
    ssd1351_fill_rect(0x40, 0x00, 0x10, SSD1351_SCREEN_HEIGHT, Red);

    // Green => Column 80~95
    ssd1351_fill_rect(0x50, 0x00, 0x10, SSD1351_SCREEN_HEIGHT, Lime);

    // Blue => Column 96~111
    ssd1351_fill_rect(0x60, 0x00, 0x10, SSD1351_SCREEN_HEIGHT, Blue);

    // Black => Column 112~127
    ssd1351_fill_rect(0x70, 0x00, 0x10, SSD1351_SCREEN_HEIGHT, Black);
}
void ssd1351_init(void)
{
    // Initialise the board interface
    init_board(g);

    // Hardware reset
    setpin_reset(g, 0);
    ets_delay_us(100000);
    setpin_reset(g, 1);
    ets_delay_us(100000);

    write_reg(g, SSD1351_SET_COMMAND_LOCK, 0x12);   // unlock OLED driver IC
    write_reg(g, SSD1351_SET_COMMAND_LOCK, 0xB1);   // make commands A1, B1, B3, BB, BE, C1 accesible in unlocked state
    write_cmd(g, SSD1351_SET_SLEEP_ON);             // sleep mode ON (display off)
    write_reg(g, SSD1351_CLOCKDIV_OSCFREQ, 0xF1);   // Front clock divider / osc freq - Osc = 0xF; div = 2
    write_reg(g, SSD1351_SET_MUX_RATIO, 127);       // set MUX ratio
    write_reg(g, SSD1351_SET_REMAP, 0b01110100);    // Set re-map / color depth
    // [0] : address increment (0: horizontal, 1: vertical, reset 0)
    // [1] : column remap (0: 0..127, 1: 127..0, reset 0)
    // [2] : color remap (0: A->B->C, 1: C->B->A, reset 0)
    // [3] : reserved
    // [4] : column scan direction (0: top->down, 1: bottom->up, reset 0)
    // [5] : odd/even split COM (0: disable, 1: enable, reset 1)
    // [6..7] : color depth (00,01: 65k, 10: 262k, 11: 262k format 2)

    write_reg(g, SSD1351_SET_DISPLAY_START, 0x00);  // set display start line - 0
    write_reg(g, SSD1351_SET_DISPLAY_OFFSET, 0x00); // set display offset - 0
    write_reg(g, SSD1351_SET_GPIO, 0x00);           // set GPIO - both HiZ, input disabled
    write_reg(g, SSD1351_SET_FUNCTION_SELECT, 0x01);    // enable internal VDD regulator
    write_reg(g, SSD1351_SET_RESET_PRECHARGE, 0x32);    // set reset / pre-charge period - phase 2: 3 DCLKs, phase 1: 5 DCLKs
    write_reg(g, SSD1351_SET_VCOMH, 0x05);          // set VComH voltage - 0.82*Vcc
    write_reg(g, SSD1351_SET_PRECHARGE, 0x17);      // set pre-charge voltage - 0.6*Vcc
    write_cmd(g, SSD1351_SET_DISPLAY_MODE_RESET);   // set display mode: reset to normal display

    write_cmd(g, SSD1351_SET_CONTRAST);             // set contrast current for A,B,C
    write_data(g, 0xC8);
    write_data(g, 0x80);
    write_data(g, 0xC8);

    write_reg(g, SSD1351_MASTER_CONTRAST_CURRENT_CONTROL, 0x0F);    // master contrast current control - no change

    write_cmd(g, SSD1351_SET_VSL);                  // set segment low voltage
    write_data(g, 0xA0);    // external VSL
    write_data(g, 0xB5);    // hard value
    write_data(g, 0x55);    // hard value

    write_reg(g, SSD1351_SET_SECOND_PRECHARGE, 0x01);   // set second pre-charge period - 1 DCLKs

    write_cmd(g, SSD1351_DISPLAY_ENHANCEMENT);      // display enhancement
    write_data(g, 0xA4);    // enhance display performance
    write_data(g, 0x00);    // fixed
    write_data(g, 0x00);    // fixed

    write_cmd(g, SSD1351_WRITE_RAM);                // write to RAM

    write_cmd(g, SSD1351_LUT_GRAYSCALE);            // set pulse width for gray scale table
    for(int i=0;i<sizeof(gray_scale_table);i++)
        write_data(g, gray_scale_table[i]);

    write_cmd(g, SSD1351_SET_SLEEP_OFF);            // sleep mode OFF (display on)
    //ssd1351_refresh_gram(0);


    /* Initialise the GDISP structure */
    // g->g.Width = SSD1351_SCREEN_WIDTH
    // g->g.Height = SSD1351_SCREEN_HEIGHT;
    // g->g.Orientation = GDISP_ROTATE_0;
    // g->g.Powermode = powerOn;
    // g->g.Backlight = GDISP_INITIAL_BACKLIGHT;
    // g->g.Contrast = GDISP_INITIAL_CONTRAST;
    
}
