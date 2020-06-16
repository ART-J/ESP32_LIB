#include "driver/spi_master.h"

#include "ssd1306.h"

//#define PIN_NUM_MISO 25
#define PIN_NUM_MOSI 18
#define PIN_NUM_CLK  5
#define PIN_NUM_CS   27

spi_device_handle_t spi;

void hspi_init(void)
{
    esp_err_t ret;
    spi_bus_config_t buscfg={
        .miso_io_num=-1,                       
        .mosi_io_num=PIN_NUM_MOSI,
        .sclk_io_num=PIN_NUM_CLK,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        .max_transfer_sz=SSD1306_HEIGHT*SSD1306_WIDTH*2
    };
    spi_device_interface_config_t devcfg={
        .clock_speed_hz=10*1000*1000,           
        .mode=0,                                
        .spics_io_num=PIN_NUM_CS,              
        .queue_size=13,                          
        .pre_cb=ssd1306_setpin_dc_callback,
        .flags=SPI_DEVICE_3WIRE | SPI_DEVICE_HALFDUPLEX
    };   
    //Initialize the SPI bus
    ret=spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    ESP_ERROR_CHECK(ret);
    //Attach the LCD to the SPI bus
    ret=spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);
}