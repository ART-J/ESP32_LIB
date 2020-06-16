/*
 * ssd1306.c
 * 
 * Created on:  2018-10-3
 *     Auther:  ChenFY  
 * 
 * driver: spi
 * pin set:
 *          --------function--------
 *          D0          PIN_NUM_CLK
 *          D1          PIN_NUM_MOSI
 *          RES         PIN_NUM_RST
 *          DC          PIN_NUM_DC
 *          CS          PIN_NUM_CS
 *          ------------------------
 * 
 */
#include <string.h>
#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "hspi.h"
#include "ssd1306.h"
#include "ssd1306_font.h"

#define PIN_NUM_DC   23
#define PIN_NUM_RST  14

/* Send a command to the LCD. Uses spi_device_polling_transmit, which waits
 * until the transfer is complete.
 *
 * Since command transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void ssd1306_write_cmd(const uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                     //Command is 8 bits
    t.tx_buffer=&cmd;               //The data is the cmd itself
    t.user=(void*)0;                //D/C needs to be set to 0
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

/* Send data to the LCD. Uses spi_device_polling_transmit, which waits until the
 * transfer is complete.
 *
 * Since data transactions are usually small, they are handled in polling
 * mode for higher speed. The overhead of interrupt transactions is more than
 * just waiting for the transaction to complete.
 */
void ssd1306_write_data(const uint8_t data)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));       //Zero out the transaction
    t.length=8;                 
    t.tx_buffer=&data;               //Data
    t.user=(void*)1;                //D/C needs to be set to 1
    ret=spi_device_polling_transmit(spi, &t);  //Transmit!
    assert(ret==ESP_OK);            //Should have had no issues.
}

//This function is called (in irq context!) just before a transmission starts. It will
//set the D/C line to the value indicated in the user field.
void ssd1306_setpin_dc_callback(spi_transaction_t *t)
{
    int dc=(int)t->user;
    gpio_set_level(PIN_NUM_DC, dc);
}

void ssd1306_clear(void)
{
    for (int y=0; y<8; y++) {
        ssd1306_write_cmd(0xB0+y);
        ssd1306_write_cmd(0x00);       //Set Lower Column Start Address 
        ssd1306_write_cmd(0x10);       //Set Higher Column Start Address     
        for (int x=0; x<128; x++)
            ssd1306_write_data(0x00);      //Page Addressing Mode will fill in the one page automaticlly
    }
}

void ssd1306_set_pos(uint8_t x, uint8_t y)
{
    ssd1306_write_cmd(0xB0+y);
    ssd1306_write_cmd((x&0x0F) | 0x01);
    ssd1306_write_cmd((x&0xF0)>>4 | 0x10);
} 

//display a char (width)8X(height)16
void ssd1306_display_char(uint8_t x, uint8_t y, char chr)
{
    uint8_t i, chrTemp;
    chrTemp=chr-' ';  
    if (x > SSD1306_WIDTH-1) {
        x=0;
        y=y+2;
    }  
    ssd1306_set_pos(x, y);
    for (i=0; i<8; i++) 
        ssd1306_write_data(chFont1608[chrTemp][i]);
    y++;
    ssd1306_set_pos(x, y);
    for (i=8; i<16; i++) 
        ssd1306_write_data(chFont1608[chrTemp][i]);
}

//display a string
void ssd1306_display_string(uint8_t x, uint8_t y, char *str)
{
    while(*str != '\0') {
        ssd1306_display_char(x, y, *str);  
        x += 8;
        if(x > 120) {
            x=0;
            y=y+2;
        }
        str++;      
    }
}

void ssd1306_init(void)
{
    hspi_init();
    //Initialize non-SPI GPIOs
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);

    //Reset the display
    gpio_set_level(PIN_NUM_RST, 0);
    ets_delay_us(100000);
    gpio_set_level(PIN_NUM_RST, 1);
    ets_delay_us(100000);

    esp_err_t ret;
    static spi_transaction_t trans[13];

    for (int x=0; x<13; x++) {
        memset(&trans[x], 0, sizeof(spi_transaction_t));
        trans[x].user=(void*)0;
        trans[x].flags=SPI_TRANS_USE_TXDATA;
    }

    trans[0].length = 4*8;
    trans[0].tx_data[0]=0x20;           ///Set Memory Addressing Mode:Page Addressing Mode
    trans[0].tx_data[1]=0x10;             
    trans[0].tx_data[2]=0x00;              
    trans[0].tx_data[3]=0x10;           

    trans[1].length = 2*8;
    trans[1].tx_data[0]=0x81;           
    trans[1].tx_data[1]=0xCF; 

    trans[2].length = 3*8;
    trans[2].tx_data[0]=0xA1;           
    trans[2].tx_data[1]=0xC8; 
    trans[2].tx_data[2]=0x40;            
    
    trans[3].length = 2*8;
    trans[3].tx_data[0]=0xA6;           
    trans[3].tx_data[1]=0xA4;

    trans[4].length = 2*8;
    trans[4].tx_data[0]=0xA8;           
    trans[4].tx_data[1]=0x3F;

    trans[5].length = 2*8;
    trans[5].tx_data[0]=0xD3;           
    trans[5].tx_data[1]=0x00;

    trans[6].length = 2*8;
    trans[6].tx_data[0]=0xD5;           
    trans[6].tx_data[1]=0x80;

    trans[7].length = 2*8;
    trans[7].tx_data[0]=0xD9;           
    trans[7].tx_data[1]=0xF1;

    trans[8].length = 2*8;
    trans[8].tx_data[0]=0xD9;           
    trans[8].tx_data[1]=0xF1;

    trans[9].length = 2*8;
    trans[9].tx_data[0]=0xDA;           
    trans[9].tx_data[1]=0x12;

    trans[10].length = 2*8;
    trans[10].tx_data[0]=0xDB;           
    trans[10].tx_data[1]=0x20;

    trans[11].length = 2*8;
    trans[11].tx_data[0]=0x8D;           
    trans[11].tx_data[1]=0x14;

    trans[12].length = 8;
    trans[12].tx_data[0]=0xAF; 

    //Queue all transactions.
    for (int x=0; x<13; x++) {
        ret=spi_device_queue_trans(spi, &trans[x], portMAX_DELAY);
        assert(ret==ESP_OK);
    }

    for (int x=0; x<13; x++) {
        spi_transaction_t* ptr;
        ret=spi_device_get_trans_result(spi, &ptr, portMAX_DELAY);
        assert(ret==ESP_OK);
        assert(ptr==trans+x);
    }


    /*ssd1306 init cmd
    ssd1306_write_cmd(0xAE);       //Set Display OFF

    ssd1306_write_cmd(0x20);       //Set Memory Addressing Mode:Page Addressing Mode
    ssd1306_write_cmd(0x10);
    ssd1306_write_cmd(0x00);       //Set Lower Column Start Address 
    ssd1306_write_cmd(0x10);       //Set Higher Column Start Address
    
    ssd1306_write_cmd(0x40);       //Set Display Start Line

    ssd1306_write_cmd(0x81);       //Set Contrast Contro
    ssd1306_write_cmd(0xCF);

    ssd1306_write_cmd(0xA1);       //column address 127 is mapped to SEG0
    ssd1306_write_cmd(0xC8);       //remapped mode. Scan from COM[N-1] to COM0
    
    ssd1306_write_cmd(0xA6);       //Set Normal Display
    ssd1306_write_cmd(0xA4);       //Output follows RAM content

    ssd1306_write_cmd(0xA8);       //Set Multiplex Ratio 
    ssd1306_write_cmd(0x3F);

    ssd1306_write_cmd(0xD3);       //Set Display Offset
    ssd1306_write_cmd(0x00);
    
    ssd1306_write_cmd(0xD5);       //Set Display Clock Divide Ratio/Oscillator Frequency
    ssd1306_write_cmd(0x80);       //Suggested value 0x80
    
    ssd1306_write_cmd(0xD9);       //Set Pre-charge Period
    ssd1306_write_cmd(0xF1);
    
    ssd1306_write_cmd(0xDA);       //Set COM Pins Hardware Configuration
    ssd1306_write_cmd(0x12);

    ssd1306_write_cmd(0xDB);       //Set V COMH Deselect Level
    ssd1306_write_cmd(0x30);       //~ 0.77 x VCC
    
    ssd1306_write_cmd(0x8D);       //set Charge Pump enable(0x10 for disable)
    ssd1306_write_cmd(0x14);

    ssd1306_write_cmd(0xAF);       // //Set Display ON*/

    ssd1306_clear();
    ssd1306_set_pos(0, 0);
}
