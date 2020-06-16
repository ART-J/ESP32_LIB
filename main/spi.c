#include "driver/spi_master.h"
#include "ssd1351.h"

#define SCREEN_PANEL_SSD1351
//#define SCREEN_PANEL_SSD1306

spi_device_handle_t hspi;

void hspi_init(void)
{
    esp_err_t ret;

    spi_bus_config_t buscfg={
        .miso_io_num=-1,
        .mosi_io_num=18,
        .sclk_io_num=5,
        .quadwp_io_num=-1,
        .quadhd_io_num=-1,
        #ifdef SCREEN_PANEL_SSD1351
        .max_transfer_sz=SSD1351_SCREEN_WIDTH*SSD1351_SCREEN_HEIGHT*2
        #elif defined(SCREEN_PANEL_SSD1306)
        .max_transfer_sz = SSD1306_HEIGHT*SSD1306_WIDTH*2
        #endif
    };
    spi_device_interface_config_t devcfg={
        .clock_speed_hz=23000000,               // Clock out at 20 MHz
        .mode=0,                                // SPI mode 0
        .spics_io_num=27,                       // CS pin
        .queue_size=6,                          // We want to be able to queue 6 transactions at a time
        #ifdef SCREEN_PANEL_SSD1351
        .pre_cb=ssd1351_setpin_dc,              // Specify pre-transfer callback to handle D/C line
        #elif defined(SCREEN_PANEL_SSD1306)
        .pre_cb=ssd1306_setpin_dc_callback,
        #endif
        .flags=SPI_DEVICE_3WIRE | SPI_DEVICE_HALFDUPLEX
    };
    // Initialize the SPI bus
    ret=spi_bus_initialize(HSPI_HOST, &buscfg, 1);
    assert(ret==ESP_OK);
    // Attach the LCD to the SPI bus
    ret=spi_bus_add_device(HSPI_HOST, &devcfg, &hspi);
    assert(ret==ESP_OK);
}