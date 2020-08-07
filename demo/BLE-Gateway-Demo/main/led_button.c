/*****************************************************************************
 * @file  led_button.c
 * @brief Main loop of led task and main loop of gpio task 
 *******************************************************************************
 Copyright 2020 GL-iNet. https://www.gl-inet.com/

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at
 
 http://www.apache.org/licenses/LICENSE-2.0
 
 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 ******************************************************************************/

#include "app.h"

static xQueueHandle gpio_evt_queue = NULL;
esp_timer_handle_t oneshot_timer_3s;
esp_timer_handle_t oneshot_timer_8s;

//LED
bool ethernet_connect;
bool wifi_connect;
int ble_connect = 0;
bool mqtt_connect;
bool use_ethernet;// true for using ethernet, false for using wifi STA mode
bool network_connect;
bool normal_working_led;
int LED_flash_level;

void gpio_task(void* arg);


static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

void gpio_init(void)
{
	gpio_config_t io_conf;
	//disable interrupt
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);

    //interrupt of rising edge
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    //bit mask of the pins
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode    
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    //change gpio intrrupt type for one pin
    gpio_set_intr_type(GPIO_INPUT_IO_BUTTON, GPIO_INTR_ANYEDGE);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(gpio_task, "gpio_task", 2048, NULL, 10, NULL);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_BUTTON, gpio_isr_handler, (void*) GPIO_INPUT_IO_BUTTON);
}

void initialise_ledTask(void)
{
	xTaskCreate(led_task, "ledTask", 1024, NULL, 5, NULL);
}

void led_task(void * pvParameter)
{
	int cnt = 0;
	int tmp_num;
	int tmp_num_1;
	while(1)
	{
		if(cnt == 10000)
		{
			cnt = 0;
		}else{
			cnt++;
			tmp_num = cnt%2;
			tmp_num_1 = cnt%3;
			// printf("cnt: %d\n", cnt%2);
		}
		
		if(normal_working_led)
		{
			gpio_set_level(GPIO_OUTPUT_IO_LED1, 0);

			if(wifi_connect)
			{
				gpio_set_level(GPIO_OUTPUT_IO_LED3, tmp_num);
				//  printf("wifi_connect\n");
			}else if(ethernet_connect){

				gpio_set_level(GPIO_OUTPUT_IO_LED3, 0);
				//  printf("ethernet_connect\n");
			}else{
				
				gpio_set_level(GPIO_OUTPUT_IO_LED3, 1);
			}

			if(ble_connect > 0)
			{
				gpio_set_level(GPIO_OUTPUT_IO_LED2, 0);
			}else if(ble_connect == 0){
				gpio_set_level(GPIO_OUTPUT_IO_LED2, 1);
			}

			vTaskDelay(800 / portTICK_PERIOD_MS);
		}else{

			if(LED_flash_level == 1)
			{
				gpio_set_level(GPIO_OUTPUT_IO_LED1, tmp_num);
				gpio_set_level(GPIO_OUTPUT_IO_LED2, tmp_num);
				gpio_set_level(GPIO_OUTPUT_IO_LED3, tmp_num);
				//  printf("LED_flash_level: 1\n");
				vTaskDelay(500 / portTICK_PERIOD_MS);
			}else if(LED_flash_level == 2){

				gpio_set_level(GPIO_OUTPUT_IO_LED1, tmp_num);
				gpio_set_level(GPIO_OUTPUT_IO_LED2, tmp_num);
				gpio_set_level(GPIO_OUTPUT_IO_LED3, tmp_num);
				//  printf("LED_flash_level: 2\n");
				vTaskDelay(100 / portTICK_PERIOD_MS);
			}else if(LED_flash_level == 3)
			{
				switch (tmp_num_1)
				{
					case 0:
						gpio_set_level(GPIO_OUTPUT_IO_LED1, 0);
						gpio_set_level(GPIO_OUTPUT_IO_LED2, 1);
						gpio_set_level(GPIO_OUTPUT_IO_LED3, 1);
						break;
					case 1:
						gpio_set_level(GPIO_OUTPUT_IO_LED1, 1);
						gpio_set_level(GPIO_OUTPUT_IO_LED2, 0);
						gpio_set_level(GPIO_OUTPUT_IO_LED3, 1);
						break;
					case 2:
						gpio_set_level(GPIO_OUTPUT_IO_LED1, 1);
						gpio_set_level(GPIO_OUTPUT_IO_LED2, 1);
						gpio_set_level(GPIO_OUTPUT_IO_LED3, 0);
						break;

					default:
						break;
				}
				vTaskDelay(300 / portTICK_PERIOD_MS);
			}else{

				normal_working_led = true;
			}
		}

	}

}

void gpio_task(void* arg)
{
    uint32_t io_num;
	BaseType_t press_key = pdFALSE;
	BaseType_t lift_key = pdFALSE;
	int backup_time = 0;

    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            printf("GPIO[%d] intr, val: %d\n", io_num, gpio_get_level(io_num));
			if (gpio_get_level(io_num) == 0) 
			{
				vTaskDelay(300 / portTICK_PERIOD_MS);
				if (gpio_get_level(io_num) == 0) 
				{
					press_key = pdTRUE;
					backup_time = system_get_time();
					ESP_ERROR_CHECK(esp_timer_start_once(oneshot_timer_3s, 3000000));
					ESP_ERROR_CHECK(esp_timer_start_once(oneshot_timer_8s, 8000000));
				}
			} else if (press_key) {
				lift_key = pdTRUE;
				backup_time = system_get_time() - backup_time;
			}

			if (press_key & lift_key) 
			{
				press_key = pdFALSE;
				lift_key = pdFALSE;
				if ((backup_time > 3000000) && (backup_time < 8000000))
				{
					printf("clear config!\n");
					clean_wifi_config();
					clean_mqtt_config();
					ESP_ERROR_CHECK(esp_timer_stop(oneshot_timer_8s));
					normal_working_led = true;
					LED_flash_level = 0;
				} else if(backup_time > 8000000){
					xEventGroupSetBits(ota_group, OTA_START_BIT);
					normal_working_led = false;
					LED_flash_level = 3;
					
				}else {

					ESP_ERROR_CHECK(esp_timer_stop(oneshot_timer_3s));
					ESP_ERROR_CHECK(esp_timer_stop(oneshot_timer_8s));
				}
			}
        }
    }
}

static void oneshot_timer1_callback(void)
{
	normal_working_led = false;
	LED_flash_level = 1;
}

static void oneshot_timer2_callback(void)
{
	normal_working_led = false;
	LED_flash_level = 2;
}

void init_timer(void)
{
	const esp_timer_create_args_t oneshot_timer_args_3s = {
            .callback = &oneshot_timer1_callback,
            /* name is optional, but may help identify the timer when debugging */
            .name = "timer1"
    };
    ESP_ERROR_CHECK(esp_timer_create(&oneshot_timer_args_3s, &oneshot_timer_3s));

	const esp_timer_create_args_t oneshot_timer_args_8s = {
            .callback = &oneshot_timer2_callback,
            /* name is optional, but may help identify the timer when debugging */
            .name = "timer2"
    };
    ESP_ERROR_CHECK(esp_timer_create(&oneshot_timer_args_8s, &oneshot_timer_8s));

}
