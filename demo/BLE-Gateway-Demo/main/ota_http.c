/*****************************************************************************
 * @file  mqtt.c
 * @brief Handle ota related events  
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

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "esp_http_client.h"
#include "esp_ota_ops.h"
#include "esp_log.h"
#include "app.h"
#include "esp_event_loop.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "freertos/FreeRTOS.h"


#define BUFFSIZE 1024
static char ota_write_data[BUFFSIZE + 1] = { 0 };
char* user_ota_url = NULL;

EventGroupHandle_t ota_group;
int OTA_START_BIT = BIT0;

void initialise_ota(void)
{
	ota_group = xEventGroupCreate();
	xTaskCreate(ota_task, "ota_task", 8192, NULL, 5, NULL);
}

void ota_task(void * pvParameter)
{
	while(1)
	{
		xEventGroupWaitBits(ota_group, OTA_START_BIT, false, true, portMAX_DELAY);
		
		esp_http_client_config_t config;
		if(!user_ota_url)
		{
			config.url = create_string("http://firmware.gl-inet.cn/iot/s10/gl-s10_app.bin", strlen("http://firmware.gl-inet.cn/iot/s10/gl-s10_app.bin"));
		}else{
			printf("user set url\n");
			config.url = user_ota_url;
		}

		esp_err_t ret = esp_http_ota(&config);
		if (ret == ESP_OK) {
			// vTaskDelay(1000 / portTICK_PERIOD_MS);
			send_OTA_status(0);
			ESP_LOGE(TAG, "Firmware upgrade successed");
			vTaskDelay(2000 / portTICK_PERIOD_MS);

			esp_restart();
		} else {
			normal_working_led = true;
			LED_flash_level = 0;
			vTaskDelay(5000 / portTICK_PERIOD_MS);
			send_OTA_status(1);
			ESP_LOGE(TAG, "Firmware upgrade failed");
			xEventGroupClearBits(ota_group, OTA_START_BIT);
		}
	}
}

static void http_cleanup(esp_http_client_handle_t client)
{
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
}

esp_err_t esp_http_ota(const esp_http_client_config_t *config)
{
    if (!config) {
        ESP_LOGE(TAG, "esp_http_client config not found");
        return ESP_ERR_INVALID_ARG;
    }

    esp_http_client_handle_t client = esp_http_client_init(config);
    if (client == NULL) {
        ESP_LOGE(TAG, "Failed to initialise HTTP connection");
        return ESP_FAIL;
    }

    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK) {
        esp_http_client_cleanup(client);
        ESP_LOGE(TAG, "Failed to open HTTP connection: %s", esp_err_to_name(err));
        return err;
    }
    esp_http_client_fetch_headers(client);

    esp_ota_handle_t update_handle = 0;
    const esp_partition_t *update_partition = NULL;
    ESP_LOGI(TAG, "Starting OTA...");
    update_partition = esp_ota_get_next_update_partition(NULL);
    if (update_partition == NULL) {
        ESP_LOGE(TAG, "Passive OTA partition not found");
        http_cleanup(client);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x",
             update_partition->subtype, update_partition->address);

    err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_begin failed, error=%d", err);
        http_cleanup(client);
        return err;
    }
    ESP_LOGI(TAG, "esp_ota_begin succeeded");
    ESP_LOGI(TAG, "Please Wait. This may take time");

    esp_err_t ota_write_err = ESP_OK;
    // const int alloc_size = (config->buffer_size > 0) ? config->buffer_size : DEFAULT_OTA_BUF_SIZE;
    // char *upgrade_data_buf = (char *)malloc(alloc_size);
    // if (!upgrade_data_buf) {
    //     ESP_LOGE(TAG, "Couldn't allocate memory to upgrade data buffer");
    //     return ESP_ERR_NO_MEM;
    // }

    int binary_file_len = 0;
    while (1) {
        int data_read = esp_http_client_read(client, ota_write_data, BUFFSIZE);
        if (data_read == 0) {
            ESP_LOGI(TAG, "Connection closed, all data received");
            break;
        }
        if (data_read < 0) {
            ESP_LOGE(TAG, "Error: SSL data read error");
            break;
        }
        if (data_read > 0) {
            ota_write_err = esp_ota_write(update_handle, (const void *) ota_write_data, data_read);
            if (ota_write_err != ESP_OK) {
                break;
            }
            binary_file_len += data_read;
            ESP_LOGD(TAG, "Written image length %d", binary_file_len);
        }
    }
    // free(upgrade_data_buf);
    http_cleanup(client); 
    ESP_LOGD(TAG, "Total binary data length writen: %d", binary_file_len);
    
    esp_err_t ota_end_err = esp_ota_end(update_handle);
    if (ota_write_err != ESP_OK) {
        ESP_LOGE(TAG, "Error: esp_ota_write failed! err=0x%d", err);
        return ota_write_err;
    } else if (ota_end_err != ESP_OK) {
        ESP_LOGE(TAG, "Error: esp_ota_end failed! err=0x%d. Image is invalid", ota_end_err);
        return ota_end_err;
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_set_boot_partition failed! err=0x%d", err);
        return err;
    }
    ESP_LOGI(TAG, "esp_ota_set_boot_partition succeeded"); 

    return ESP_OK;
}
