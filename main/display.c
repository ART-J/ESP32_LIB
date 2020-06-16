#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ssd1306.h"

void ssd1306_task(void *pvParameter)
{
    while(1) {
        ssd1306_display_string(0, 0, "1s23d45");
        vTaskDelay(1000 / portTICK_RATE_MS);
        ssd1306_display_char(0, 0, '3');
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}


