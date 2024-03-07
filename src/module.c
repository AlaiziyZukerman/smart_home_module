#include <string.h>
#include <stdlib.h>

#include "module.h"
#include <zephyr/drivers/gpio.h>
#ifdef EMULATE_SENSORS
#include "zephyr/random/random.h"
#endif

char my_msgq_buffer[10 * sizeof(struct data_item_type)];
struct k_msgq my_msgq;

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
bool tx_data_format = 0;

mdl_msgq_init();
mdl_msgq_get();
mdl_msgq_put();
mdl_sleep();
mdl_console_send_msg();
mdl_uart_irq_update();
mdl_uart_irq_rx_ready();
mdl_uart_fifo_read();
mdl_uart_send_byte();
#ifdef EMULATE_SENSORS
mdl_random();
#endif

void sensor_data_send(void *d0, void *d1, void *d2) {
	
    while(1) {
		struct data_item_type q_dat;
		if (k_msgq_get(&my_msgq, &q_dat, K_NO_WAIT)) {
			k_sleep(K_MSEC(100));
			continue;
		}
		else{
			if(tx_data_format){
			printf("sensor: %u; ", q_dat.id);
			printf("tem: %u;\n", q_dat.temp);				
			}
			else {
			printf("%u:", q_dat.id);
			printf("%u;", q_dat.temp);				
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
	#ifdef EMULATE_SENSORS
		 static int sensor_number;
		 uint8_t res;
		 k_sleep(K_MSEC(1));
		 sys_rand_get(&res, sizeof(res));
		result->id = sensor_number;
		result->temp = (uint32_t) res;
		sensor_number++;
	#else
		// real sensor data request
	#endif
}

void sensor_pool (void *d0, void *d1, void *d2){
	char tx_buf[MSG_SIZE];

	if (!device_is_ready(uart_dev)) {
		printk("UART device not found!");
		return 0;
	}

	/* configure interrupt and callback to receive data */
	 uart_irq_rx_enable(uart_dev);
	 int ret = uart_irq_callback_user_data_set(uart_dev, serial_cb, NULL);
	if (ret < 0) {
		if (ret == -ENOTSUP) {
			printk("Interrupt-driven UART API support not enabled\n");
		} else if (ret == -ENOSYS) {
			printk("UART device does not support interrupt-driven API\n");
		} else {
			printk("Error setting UART callback: %d\n", ret);
		}
		return 0;
	}
	uart_irq_rx_enable(uart_dev);

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
			}
			else if (0 == strcmp (tx_buf, command_read)){
				tx_data_state = 1;
				struct data_item_type dat; 
				dat.id = 1;
				k_msgq_put(&my_msgq, &dat, K_NO_WAIT);			
			}
			else if (0 == strcmp (tx_buf, command_stop)){
				tx_data_state = 0;
				struct data_item_type dat; 
				dat.id = 0;
				k_msgq_put(&my_msgq, &dat, K_NO_WAIT);		
			}
			else if(0 == strcmp (tx_buf, command_toggle)){
				tx_data_format = !tx_data_format;
			}
			else if(0 == strcmp (tx_buf, command_status)){

			}
		}

		if(tx_data_state){
			//put message to queue for transmit data
			struct data_item_type req_result;
			sensor_dete_req(&req_result);
			k_msgq_put(&my_msgq, &req_result, K_NO_WAIT);
		}		
	}
	return 0;
}
