<<<<<<< HEAD
#ifndef __FT6336U_DRIVER_H
#define __FT6336U_DRIVER_H

#include "esp_err.h"
#include "driver/gpio.h"

esp_err_t ft6336u_driver(gpio_num_t sda,gpio_num_t scl);

void ft6335u_read(int16_t* x,int16_t* y,int* state);

void ft6336u_int_info(bool flag);
#endif
=======
#ifndef __FT6336U_DRIVER_H
#define __FT6336U_DRIVER_H

#include "esp_err.h"
#include "driver/gpio.h"

esp_err_t ft6336u_driver(gpio_num_t sda,gpio_num_t scl);

void ft6335u_read(int16_t* x,int16_t* y,int* state);

void ft6336u_int_info(bool flag);
#endif
>>>>>>> 81fc6d11b1a3aa3674e1ca4f6c48bf0d832d09d3
