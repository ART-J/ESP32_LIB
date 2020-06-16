/*
 * spi.c
 *
 *  Created on: 2018-02-10 16:37
 *      Author: Jack Chen <redchenjs@live.com>
 */

#ifndef __HSPI_H__
#define __HSPI_H__

#include "driver/spi_master.h"

extern spi_device_handle_t hspi;

extern void hspi_init(void);

#endif /* INC_DEVICE_SPI_C_ */
