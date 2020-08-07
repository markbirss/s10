/*****************************************************************************
 * @file  ble_execute.c
 * @brief Handling ble commands
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

QueueHandle_t ble_cmd_queue = NULL;
static char* ble_cmd = NULL;

static void execute_ble_task(void * pvParameter);
static void opcode_handle(char* value);

void initialise_ble_executeTask(void)
{
	xTaskCreate(execute_ble_task, "ble_executeTask", 10240, NULL, 5, NULL);
}

static void execute_ble_task(void * pvParameter)
{
	ble_cmd_queue = xQueueCreate(CMD_QUEUE_LEN, (sizeof(char*)));
	if(ble_cmd_queue == NULL)
	{
		printf("create ble cmd queue error!\n");
		return;
	}else{
		printf("ble cmd list create\n");
	}
	
	while(1)
	{
		if(xQueueReceive(ble_cmd_queue, &ble_cmd, portMAX_DELAY))
		{
			opcode_handle(ble_cmd);

			free(ble_cmd);
			ble_cmd = NULL;
		}
	}
}

static void opcode_handle(char* value)
{
    if(value[bMagicNumber] != 0xFE)
        return;
		
    uint16_t len = (value[nLength] << 8) + value[nLength+1];
	uint8_t cmdID = value[nCmdId];
    ESP_LOGI(TAG, "opcode %02x\n",cmdID);

    switch(cmdID)
	{
		case SET_WIFI_CONFIG://set wifi ssid
			set_wifi_config(&value[BODY],len-4);
			break;
		case RESTART_WIFI://start wifi
			restart_wifi();
			break;
		case START_WIFI:
			start_wifi();
			break;
		case STOP_WIFI:
			stop_wifi();
			break;
		case CLEAR_WIFI_CONFIG://clean wifi config
			clean_wifi_config();
			break;
		case GET_NETWORK_STATE://get wifi config
			get_network_state();
			break;

		case SET_MQTT_URI:
			set_mqtt_uri(&value[BODY],len-4);
			break;
		case START_MQTT://start mqtt
			start_mqtt();
			
			break;
		case STOP_MQTT://stop mqtt
			stop_mqtt();
			break;
		case RESTART_MQTT://restart mqtt
			restart_mqtt();
			break;
		case GET_MQTT_CONFIG://get mqtt config
			get_mqtt_config();
			break;
		case CLEAR_MQTT_CONFIG://clean mqtt config
			clean_mqtt_config();
			break;
		case MQTT_SUBSCRIBE://subscribe
			subscribe_mqtt(&value[BODY],len-4);
			break;
		case MQTT_UNSUBSCRIBE://unsubscribe
			unsubscribe_mqtt(&value[BODY],len-4);
			break;
		case MQTT_PUBLISH://publish
			publish_mqtt(&value[BODY],len-4);
			break;
		
		case GET_VERSION://get current version
			get_current_version();
			break;
		case SET_OTA_URL:
			set_OTA_url(&value[BODY],len-4);
			break;

		case RESTART://restart 
			esp_restart();
			break;
		case GET_MAC:
			get_mac();
			break;
		// case GET_SN:
		// 	get_sn();
		// 	break;
		// case GET_DEVICE_ID:
		// 	get_device_id();
		// 	break;
		default:
			break;
    }  
    send_ble_notify();
}

