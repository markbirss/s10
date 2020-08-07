/*****************************************************************************
 * @file  mqtt.c
 * @brief Handle mqtt related events  
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


esp_mqtt_client_config_t mqtt_cfg;
esp_mqtt_client_handle_t mqtt_client;

void mqtt_app_start(void)
{
	xEventGroupWaitBits(network_group, NETWORK_CONNECTED_BIT,
					false, true, portMAX_DELAY);
	bool mqtt_save = false;
	mqtt_connect = false;
	if(!read_mqtt_config_from_nvs())
	{
    	mqtt_cfg.uri = create_string("mqtt://username:password@127.0.0.1:61613", strlen("mqtt://username:password@127.0.0.1:61613"));
	}
    mqtt_cfg.event_handle = mqtt_event_handler;
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(mqtt_client);
}

void mqtt_notify_pack(esp_mqtt_event_handle_t event)
{
    uint16_t len = event->topic_len + event->data_len;
    if(len > CMD_MAX_LEN)
    {
        ESP_LOGI(TAG, "Error: mqtt msg too long\n");
        return ;
    }

	char tmpBuf[1024];
	memcpy(tmpBuf,event->topic,event->topic_len);
	tmpBuf[event->topic_len] = '@';
	memcpy(&tmpBuf[event->topic_len+1],event->data,event->data_len);
	create_return_notify(MQTT_SUBSCRIBE_MES, tmpBuf, len+1);
    return ;
}

esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
			mqtt_connect = true;
			write_mqtt_config_to_nvs();

            break;
        case MQTT_EVENT_DISCONNECTED:
			mqtt_connect = false;
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
			
            mqtt_notify_pack(event);
            send_ble_notify();
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    return ESP_OK;
}
