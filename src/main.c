#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include "module.h"
#include <zephyr/logging/log.h>

 #define SLEEP_TIME_MS   1

#define MY_STACK_SIZE 500
#define MY_PRIORITY 5

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

 extern char sensor_data_send_msgq_buffer[10 * sizeof(struct data_item_type)];
 extern struct k_msgq sensor_data_send_msgq;

struct k_thread sensor_data_send_tid, sensor_pool_tid;

K_THREAD_STACK_DEFINE(sensor_data_send_stack_area, MY_STACK_SIZE);
K_THREAD_STACK_DEFINE(sensor_pool_stack_area, MY_STACK_SIZE);

int main(void)
{
	LOG_INF("main() start");

	LOG_INF("m_queue init: sensor_data_send_msgq");
	k_msgq_init(&sensor_data_send_msgq, sensor_data_send_msgq_buffer, sizeof(struct data_item_type), 10);

	LOG_INF("thread init: sensor_data_send_tid");
	k_thread_create(&sensor_data_send_tid, sensor_data_send_stack_area, K_THREAD_STACK_SIZEOF(sensor_data_send_stack_area),
					sensor_data_send, NULL, NULL, NULL, MY_PRIORITY, 0, K_NO_WAIT);

	LOG_INF("thread init: sensor_pool_tid");
	k_thread_create(&sensor_pool_tid, sensor_pool_stack_area, K_THREAD_STACK_SIZEOF(sensor_pool_stack_area),
					sensor_pool, NULL, NULL, NULL, MY_PRIORITY, 0, K_NO_WAIT);

	while (1) {

	 	k_msleep(SLEEP_TIME_MS);
	}
	return 0;
}

