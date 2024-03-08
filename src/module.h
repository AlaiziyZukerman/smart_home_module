#ifndef _MODULE_H
#define _MODULE_H

#include <stdint.h>
#include "zephyr/kernel.h"
#include "zephyr/drivers/gpio.h"
#include <zephyr/drivers/uart.h>

#define FREERTOS            0
#define EMULATE_SENSORS     1

#define MY_STACK_SIZE       500
#define MY_PRIORITY         5

#define ANIMATE_PING_STACK  1024
#define SENSOR_POOL_STACK   1024

struct data_item_type {
    uint32_t id ;
    uint32_t temp;
};

void sensor_data_send(void *d0, void *d1, void *d2);
void sensor_pool (void *d0, void *d1, void *d2);
void serial_cb(const struct device *dev, void *user_data);
void sensor_dete_req(struct data_item_type *result);

#endif //_MODULE_H
