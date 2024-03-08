/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include "module.h"
// #include <zephyr/logging/log.h>
/* 1000 msec = 1 sec */
 #define SLEEP_TIME_MS   1000

//LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

 extern char sensor_data_send_msgq_buffer[10 * sizeof(struct data_item_type)];
 extern struct k_msgq sensor_data_send_msgq;


const struct device *uart1 = DEVICE_DT_GET(DT_NODELABEL(uart1));

struct uart_config uart_cfg = {
	.baudrate = 115200,
	.parity = UART_CFG_PARITY_NONE,
	.stop_bits = UART_CFG_STOP_BITS_1,
	.flow_ctrl = UART_CFG_FLOW_CTRL_NONE,
	.data_bits = UART_CFG_DATA_BITS_8,
};


#define MY_STACK_SIZE 500
#define MY_PRIORITY 5

struct k_thread sensor_data_send_tid, sensor_pool_tid;

K_THREAD_STACK_DEFINE(sensor_data_send_stack_area, MY_STACK_SIZE);
K_THREAD_STACK_DEFINE(sensor_pool_stack_area, MY_STACK_SIZE);

int main(void)
{
	//LOG_INF("main() start");
	
	uart_configure(uart1, &uart_cfg);
	//LOG_INF("thread init: sensor_data_send_tid");
	k_thread_create(&sensor_data_send_tid, sensor_data_send_stack_area, K_THREAD_STACK_SIZEOF(sensor_data_send_stack_area),
					sensor_data_send, NULL, NULL, NULL, MY_PRIORITY, 0, K_NO_WAIT);

	//LOG_INF("thread init: sensor_pool_tid");
	k_thread_create(&sensor_pool_tid, sensor_pool_stack_area, K_THREAD_STACK_SIZEOF(sensor_pool_stack_area),
					sensor_pool, NULL, NULL, NULL, MY_PRIORITY, 0, K_NO_WAIT);

	//LOG_INF("m_queue init: sensor_data_send_msgq");
	k_msgq_init(&sensor_data_send_msgq, sensor_data_send_msgq_buffer, sizeof(struct data_item_type), 10);
	//k_thread_suspend(animate_ping_tid);
	// int ret;
	// bool led_state = true;
	//k_thread_start(animate_ping_tid);
	//k_thread_start(sensor_pool);
	// message_queue_put_handler (0, 0);
	// k_sleep(K_MSEC(700));
	// message_queue_put_handler (0, 0);
	// char in_buff [50];
	// char send[] = "I'm here!";
	// int msg_len = strlen(send);

	// for (int i = 0; i < msg_len; i++) {
	// 	uart_poll_out(uart1, send[i]);
	// }
	//LOG_INF("main() init complete");
	while (1) {
		// for (int i = 0; i < msg_len; i++) {
		// 	uart_poll_out(uart1, send[i]);
		// }

	 	k_msleep(SLEEP_TIME_MS);
	}
	return 0;
}

