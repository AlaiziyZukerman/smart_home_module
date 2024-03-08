#include <string.h>
#include <stdlib.h>

#include "module.h"
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#ifdef EMULATE_SENSORS
#include "zephyr/random/random.h"
#endif
LOG_MODULE_REGISTER(module, LOG_LEVEL_INF);
char sensor_data_send_msgq_buffer[10 * sizeof(struct data_item_type)];
struct k_msgq sensor_data_send_msgq;
const struct device *uart1 = DEVICE_DT_GET(DT_NODELABEL(uart1));
//useless
// void message_queue_put_handler (uint32_t *id, uint32_t *temp){
// 	struct data_item_type dat;
// 	dat.id = &id;
// 	dat.temp = &temp;
// 	k_msgq_put(&my_msgq, &dat, K_NO_WAIT);
// }

char command_time	[] = "time:";
char command_read	[] = "read";
char command_stop	[] = "stop";
char command_toggle	[] = "toggle";
char command_status	[] = "status";

int pooling_delay_time = 1000;
bool tx_data_state = 0;
bool tx_data_format = 1;

// mdl_msgq_init();
// mdl_msgq_get();
// mdl_msgq_put();
// mdl_sleep();
// mdl_console_send_msg();
// mdl_uart_init();
// mdl_uart_irq_update();
// mdl_uart_irq_rx_ready();
// mdl_uart_fifo_read();
// mdl_uart_send_byte();
// #ifdef EMULATE_SENSORS
// mdl_random();
// #endif
// mdl_log();

void mdl_uart_init(void){
	#if FREERTOS

	#else
		// const struct device *uart1 = DEVICE_DT_GET(DT_NODELABEL(uart1));
		
		struct uart_config uart_cfg = {
		.baudrate = 115200,
		.parity = UART_CFG_PARITY_NONE,
		.stop_bits = UART_CFG_STOP_BITS_1,
		.flow_ctrl = UART_CFG_FLOW_CTRL_NONE,
		.data_bits = UART_CFG_DATA_BITS_8,
		};

		uart_configure(uart1, &uart_cfg);
	#endif
}


void sensor_data_send(void *d0, void *d1, void *d2) {
	mdl_uart_init();
	LOG_INF("thread init: sensor_data_send_tid->complete");
    while(1) {
		struct data_item_type q_dat;
		if (k_msgq_get(&sensor_data_send_msgq, &q_dat, K_NO_WAIT)) {
			k_sleep(K_MSEC(100));
			continue;
		}
		else{
			if(tx_data_format){
				char mmm [20];

				sprintf(mmm, "sensor: %u; ", q_dat.id);
				int msg_len = strlen(mmm);

				for (int i = 0; i < msg_len; i++) {
					uart_poll_out(uart1, mmm[i]);
				}

				sprintf(mmm, "temp: %u;\r\n", q_dat.temp);
				msg_len = strlen(mmm);

				for (int i = 0; i < msg_len; i++) {
					uart_poll_out(uart1, mmm[i]);
				}
			// printf("sensor: %u; ", q_dat.id);
			// printf("tem: %u;\n", q_dat.temp);				
			}
			else {
				// char myass[] = "myblackass";
				char mmm [10];

				sprintf(mmm, "%u:", q_dat.id);
				int msg_len = strlen(mmm);

				for (int i = 0; i < msg_len; i++) {
					uart_poll_out(uart1, mmm[i]);
				}

				sprintf(mmm, "%u;", q_dat.temp);
				msg_len = strlen(mmm);

				for (int i = 0; i < msg_len; i++) {
					uart_poll_out(uart1, mmm[i]);
				}
				
			}
			

		}
		
    }
}

#define UART_DEVICE_NODE DT_CHOSEN(zephyr_shell_uart)
#define MSG_SIZE 10

K_MSGQ_DEFINE(uart_msgq, MSG_SIZE, 10, 4);

static const struct device *const uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

static char rx_buf[MSG_SIZE];
static int rx_buf_pos;

void serial_cb(const struct device *dev, void *user_data)
{
	uint8_t c;

	if (!uart_irq_update(uart_dev)) {
		return;
	}

	if (!uart_irq_rx_ready(uart_dev)) {
		return;
	}

	/* read until FIFO empty */
	while (uart_fifo_read(uart_dev, &c, 1) == 1) {
		if ((c == '\n' || c == '\r') && rx_buf_pos > 0) {
			/* terminate string */
			rx_buf[rx_buf_pos] = '\0';

			/* if queue is full, message is silently dropped */
			k_msgq_put(&uart_msgq, &rx_buf, K_NO_WAIT);

			/* reset the buffer (it was copied to the msgq) */
			rx_buf_pos = 0;
		} else if (rx_buf_pos < (sizeof(rx_buf) - 1)) {
			rx_buf[rx_buf_pos++] = c;
		}
		/* else: characters beyond buffer size are dropped */
	}
}

void print_uart(char *buf)
{
	int msg_len = strlen(buf);

	for (int i = 0; i < msg_len; i++) {
		uart_poll_out(uart_dev, buf[i]);
	}
}
void sensor_dete_req(struct data_item_type *result){
	#if EMULATE_SENSORS
		static int sensor_number;
		uint8_t res = 0;
		k_sleep(K_MSEC(1));
		sys_rand_get(&res, sizeof(res));
		result->id = sensor_number;
		result->temp = (uint32_t) res;
		sensor_number++;
		if(sensor_number > 257) sensor_number = 0;
	#else
		// real sensor data request
	#endif
}

void sensor_pool (void *d0, void *d1, void *d2){
	char tx_buf[MSG_SIZE];

	if (!device_is_ready(uart_dev)) {
		LOG_ERR("UART device not found!");
		return 0;
	}

	/* configure interrupt and callback to receive data */
	 uart_irq_rx_enable(uart_dev);
	 int ret = uart_irq_callback_user_data_set(uart_dev, serial_cb, NULL);
	if (ret < 0) {
		if (ret == -ENOTSUP) {
			LOG_ERR("Interrupt-driven UART API support not enabled\n"); //printk
		} else if (ret == -ENOSYS) {
			LOG_ERR("UART device does not support interrupt-driven API\n");
		} else {
			LOG_ERR("Error setting UART callback: %d\n", ret);
		}
		return 0;
	}
	uart_irq_rx_enable(uart_dev);
	LOG_INF("thread init: sensor_pool_tid->complete");
	while (1) {

		if (k_msgq_get(&uart_msgq, &tx_buf, K_NO_WAIT)){
			//recive parameters and handl
			k_sleep(K_MSEC(pooling_delay_time));
		}
		else {
			if (0 == strncmp(tx_buf, command_time, 5)){
				char time [strlen(&tx_buf) - 5];
				strncpy(&time, &tx_buf[5], strlen(&tx_buf) - 5);
				pooling_delay_time = atoi(time);
				LOG_INF("command accepted: pooling time->%u msec", pooling_delay_time);
			}
			else if (0 == strcmp (tx_buf, command_read)){
				tx_data_state = 1;
				LOG_INF("command accepted: reading start");	
			}
			else if (0 == strcmp (tx_buf, command_stop)){
				tx_data_state = 0;	
				LOG_INF("command accepted: reading stop");	
			}
			else if(0 == strcmp (tx_buf, command_toggle)){
				tx_data_format = !tx_data_format;
				LOG_INF("command accepted: toggle data format");	
			}
			else if(0 == strcmp (tx_buf, command_status)){
				LOG_INF("status: reading->%s; fotmat->%s; time->%u msec;", tx_data_state ? ("start"):("stop"),
						tx_data_format ? ("full"):("bin"), pooling_delay_time);	
			}
			else{
				LOG_INF("illegal command");
			}
		}

		if(tx_data_state){
			//put message to queue for transmit data
			struct data_item_type req_result;
			sensor_dete_req(&req_result);
			k_msgq_put(&sensor_data_send_msgq, &req_result, K_NO_WAIT);
		}		
	}
	return 0;
}
