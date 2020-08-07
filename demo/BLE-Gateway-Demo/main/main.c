/*****************************************************************************
 * @file  main.c
 * @brief Main program body
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

// #define EXAMPLE_ESP_MAXIMUM_RETRY  3
// static int s_retry_num = 0;
bool WIFI_SWITCH;

char network_ip[50];
char sta_mac[13];

// not used
#define UART1_TXD      (GPIO_NUM_4)
#define UART1_RXD      (GPIO_NUM_16)
#define UART1_RTS      (UART_PIN_NO_CHANGE)
#define UART1_CTS      (UART_PIN_NO_CHANGE)
#define BUF_SIZE (10240)

/*Network event handler*/
esp_err_t network_event_handler(void *ctx, system_event_t *event)
{
	tcpip_adapter_ip_info_t ip;
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
		
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "got ip:%s",
                 ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
		network_connect = true;
		wifi_connect = true;
		strcpy(network_ip, ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        // s_retry_num = 0;
		write_wifi_config_to_nvs();
		xEventGroupSetBits(network_group, NETWORK_CONNECTED_BIT);
		xEventGroupSetBits(network_group, WIFI_CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        {
			xEventGroupClearBits(network_group, NETWORK_CONNECTED_BIT);
			xEventGroupClearBits(network_group, WIFI_CONNECTED_BIT);
			network_connect = false;
			wifi_connect = false;
			memset(network_ip, 0, sizeof(network_ip));
			ESP_LOGI(TAG,"connect to the AP fail\n");
            //if ((s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) && (WIFI_SWITCH)) 
			if(WIFI_SWITCH)
			{
				vTaskDelay(1000 / portTICK_PERIOD_MS);
                esp_wifi_connect();
                // s_retry_num++;
                ESP_LOGI(TAG,"retry to connect to the AP");
            }

            break;
        }
	case SYSTEM_EVENT_ETH_CONNECTED:
        ESP_LOGI(TAG, "Ethernet Link Up");
		use_ethernet = true;
		wifi_connect = false;
		esp_wifi_stop();
        break;
    case SYSTEM_EVENT_ETH_DISCONNECTED:
        ESP_LOGI(TAG, "Ethernet Link Down");
		xEventGroupClearBits(network_group, NETWORK_CONNECTED_BIT);
		network_connect = false;
		ethernet_connect = false;
		memset(network_ip, 0, sizeof(network_ip));
		use_ethernet = false;
		esp_wifi_start();
        break;
    case SYSTEM_EVENT_ETH_START:
        ESP_LOGI(TAG, "Ethernet Started");
        break;
    case SYSTEM_EVENT_ETH_GOT_IP:
		xEventGroupSetBits(network_group, NETWORK_CONNECTED_BIT);
        memset(&ip, 0, sizeof(tcpip_adapter_ip_info_t));
        ESP_ERROR_CHECK(tcpip_adapter_get_ip_info(ESP_IF_ETH, &ip));
        ESP_LOGI(TAG, "Ethernet Got IP Addr");
        ESP_LOGI(TAG, "~~~~~~~~~~~");
        ESP_LOGI(TAG, "ETHIP:" IPSTR, IP2STR(&ip.ip));
        ESP_LOGI(TAG, "ETHMASK:" IPSTR, IP2STR(&ip.netmask));
        ESP_LOGI(TAG, "ETHGW:" IPSTR, IP2STR(&ip.gw));
        ESP_LOGI(TAG, "~~~~~~~~~~~");
		network_connect = true;
		ethernet_connect = true;
		strcpy(network_ip, ip4addr_ntoa(&ip.ip));
        break;
    case SYSTEM_EVENT_ETH_STOP:
        ESP_LOGI(TAG, "Ethernet Stopped");
        break;

    default:
        break;
    }
    return ESP_OK;
}


void app_main()
{
    esp_err_t ret;

    // Initialize NVS.
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    //init ble
	init_ble();

	//init gpio
	gpio_init();

	//led init
	gpio_set_level(GPIO_OUTPUT_IO_LED1, 0);//power led on
	ble_connect = 0;
	gpio_set_level(GPIO_OUTPUT_IO_LED2, 1);//bluetooth led off
	wifi_connect = false;
	ethernet_connect = false;
	gpio_set_level(GPIO_OUTPUT_IO_LED3, 1);//network led off
	normal_working_led = true;

	//init timer
	init_timer();

	//init tcp/ip
	tcpip_adapter_init();

	//init wifi sta
    wifi_init_sta();
    uint8_t tmp_sta_mac[6];
	esp_wifi_get_mac(ESP_IF_WIFI_STA, tmp_sta_mac);
	sprintf(sta_mac, "%02x%02x%02x%02x%02x%02x", tmp_sta_mac[0], tmp_sta_mac[1], tmp_sta_mac[2], tmp_sta_mac[3], tmp_sta_mac[4], tmp_sta_mac[5]);
	sta_mac[12] = '\0';

	//init ethernet
	initialise_eth();

	//init ota
	initialise_ota();

	//init ble_executeTask
	initialise_ble_executeTask();

	//init ledTask
	initialise_ledTask();

	//init transpondTask
	init_transpondTask();

	//init time, need network connected
	// init_time();
	
	//init mqtt client, need network connected
	mqtt_app_start();

	//init scan
	init_ble_scan_task();
    return;
}
