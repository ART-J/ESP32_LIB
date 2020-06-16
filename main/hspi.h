/*
 * hspi.c
 * 
 * Created on:  2018-10-3
 *     Auther:  ChenFY  
 * 
 */
#ifndef __HSPI_H__
#define __HSPI_H__

#include "driver/spi_master.h"

extern spi_device_handle_t spi;

extern void hspi_init(void);

#endif