#ifndef INC_DRIVER_SSD1351_H_
#define INC_DRIVER_SSD1351_H_

#include <stdint.h>
#include "hspi.h"

#define SSD1351_SCREEN_WIDTH   128
#define SSD1351_SCREEN_HEIGHT  128

#define SSD1351_SET_COLUMN_ADDRESS              0x15
#define SSD1351_SET_ROW_ADDRESS                 0x75
#define SSD1351_WRITE_RAM                       0x5C
#define SSD1351_READ_RAM                        0x5D
#define SSD1351_SET_REMAP                       0xA0

#define SSD1351_SET_DISPLAY_START               0xA1
#define SSD1351_SET_DISPLAY_OFFSET              0xA2
#define SSD1351_SET_DISPLAY_MODE_ALL_OFF        0xA4
#define SSD1351_SET_DISPLAY_MODE_ALL_ON         0xA5
#define SSD1351_SET_DISPLAY_MODE_RESET          0xA6
#define SSD1351_SET_DISPLAY_MODE_INVERT         0xA7
#define SSD1351_SET_FUNCTION_SELECT             0xAB
#define SSD1351_SET_SLEEP_ON                    0xAE
#define SSD1351_SET_SLEEP_OFF                   0xAF
#define SSD1351_SET_RESET_PRECHARGE             0xB1

#define SSD1351_DISPLAY_ENHANCEMENT             0xB2
#define SSD1351_CLOCKDIV_OSCFREQ                0xB3
#define SSD1351_SET_VSL                         0xB4
#define SSD1351_SET_GPIO                        0xB5
#define SSD1351_SET_SECOND_PRECHARGE            0xB6

#define SSD1351_LUT_GRAYSCALE                   0xB8
#define SSD1351_USE_BUILTIN_LUT                 0xB9
#define SSD1351_SET_PRECHARGE                   0xBB
#define SSD1351_SET_VCOMH                       0xBE

#define SSD1351_SET_CONTRAST                    0xC1
#define SSD1351_MASTER_CONTRAST_CURRENT_CONTROL 0xC7
#define SSD1351_SET_MUX_RATIO                   0xCA
#define SSD1351_SET_COMMAND_LOCK                0xFD

enum ssd1351_graphic_acceleration_command_table {
    HORIZONTAL_SCROLLING_SETUP  = 0x96,
    DEACTIVATE_SCROLLING        = 0x9E,
    ACTIVATE_SCROLLING          = 0x9F
};
enum ssd1351_scroll_direction {
    Left    = 0x01,
    Right   = 0x81
};

enum ssd1351_scroll_interval {
    Fast    = 0x00,
    Normal  = 0x01,
    Slow    = 0x02,
    Slowest = 0x03
};

extern void ssd1351_init_board(void);
extern void ssd1351_setpin_dc(spi_transaction_t *);
extern void ssd1351_setpin_reset(uint8_t rst);

extern void ssd1351_write_cmd(uint8_t cmd);
extern void ssd1351_write_data(uint8_t data);
extern void ssd1351_refresh_gram(uint8_t *gram);

extern void ssd1351_draw_point(unsigned char chXpos, unsigned char chYpos, unsigned int hwColor);
extern void ssd1351_draw_line(unsigned char chXpos0, unsigned char chYpos0, unsigned char chXpos1, unsigned char chYpos1, unsigned int hwColor);
extern void ssd1351_draw_h_line(unsigned char chXpos, unsigned char chYpos, unsigned char chWidth, unsigned int hwColor);
extern void ssd1351_draw_v_line(unsigned char chXpos, unsigned char chYpos, unsigned char chHeight, unsigned int hwColor);

extern void ssd1351_draw_column(unsigned char chXpos, unsigned char chYpos, unsigned char chHeight, unsigned int hwColor0, unsigned int hwColor1);

extern void ssd1351_draw_rect(unsigned char chXpos, unsigned char chYpos, unsigned char chWidth, unsigned char chHeight, unsigned int hwColor);
extern void ssd1351_draw_circle(unsigned char chXpos, unsigned char chYpos, unsigned char chRadius, unsigned int hwColor);

extern void ssd1351_draw_mono_bitmap(unsigned char chXpos, unsigned char chYpos, const unsigned char *pchBmp, unsigned char chWidth, unsigned char chHeight, unsigned int hwForeColor, unsigned int hwBackColor);
extern void ssd1351_draw_64k_bitmap(unsigned char chXpos, unsigned char chYpos, const unsigned char *pchBmp, unsigned char chWidth, unsigned char chHeight);

extern void ssd1351_fill_rect(unsigned char chXpos, unsigned char chYpos, unsigned char chWidth, unsigned char chHeight, unsigned int hwColor);
extern void ssd1351_fill_gram(unsigned int hwColor);

extern void ssd1351_clear_rect(unsigned char chXpos, unsigned char chYpos, unsigned char chWidth, unsigned char chHeight);
extern void ssd1351_clear_gram(void);

extern void ssd1351_display_char(unsigned char chXpos, unsigned char chYpos, unsigned char chChr, unsigned char chFontIndex, unsigned int hwForeColor, unsigned int hwBackColor);
extern void ssd1351_display_num(unsigned char chXpos, unsigned char chYpos, unsigned long chNum, unsigned char chLen, unsigned char chFontIndex, unsigned int hwForeColor, unsigned int hwBackColor);
extern void ssd1351_display_string(unsigned char chXpos, unsigned char chYpos, const char *pchString, unsigned char chFontIndex, unsigned int hwForeColor, unsigned int hwBackColor);

extern void ssd1351_horizontal_scrolling(unsigned char chYpos, unsigned char chHeight, unsigned char chDirection, unsigned char chInterval);
extern void ssd1351_deactivate_scrolling(void);

extern void ssd1351_show_checkerboard(void);
extern void ssd1351_show_rainbow(void);

extern void ssd1351_init(void);


#endif /* INC_DRIVER_SSD1351_H_ */