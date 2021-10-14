/*****************************************************************************
 * @file  ble_scan_task.c
 * @brief Main loop of transpond task 
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

#define SCAN_RESULT_MAX 50
#define MQTT_ADV_TOPIC "gl-s10-scanResult"
QueueHandle_t scan_result_queue = NULL;
uint8_t tmp_sta_mac[6];
uint8_t tmp_ble_mac[6];

void init_transpondTask(void)
{
    printf("init_transpondTask\n");
    xTaskCreate(transpond_task, "transpondTask", 4096, NULL, 5, NULL);
}

void transpond_task(void* pvParameter)
{
    printf("transpond_task\n");
    scan_result_queue = xQueueCreate(SCAN_RESULT_MAX, (sizeof(esp_ble_gap_cb_param_t*)));

    if (scan_result_queue == NULL) {
        printf("create transpond queue error!\n");
        return;
    } else {
        printf("transpond list create\n");
    }

    esp_read_mac(tmp_sta_mac, ESP_MAC_WIFI_STA);
    esp_read_mac(tmp_ble_mac, ESP_MAC_BT);

    esp_ble_gap_cb_param_t* scan_result = NULL;
    while (1) {
        if (xQueueReceive(scan_result_queue, &scan_result, portMAX_DELAY)) {
            send2cloud(scan_result);
            // ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());

            free(scan_result);
            scan_result = NULL;
        }
    }
}

void send2cloud(esp_ble_gap_cb_param_t* scan_result)
{
    cJSON* data = NULL;
    bool need_send = false;
    data = cJSON_CreateObject();

    // char time[64];
    // long now_time;
    // now_time = get_time();
    // ltoa(now_time, time);
    // cJSON_AddItemToObject(data, "time", cJSON_CreateString(time));

    // uint8_t *tmp_adv_name = NULL;
    // uint8_t adv_name_len = 0;
    // tmp_adv_name = esp_ble_resolve_adv_data(scan_result->scan_rst.ble_adv,
    //                                             ESP_BLE_AD_TYPE_NAME_CMPL, &adv_name_len);
    // char adv_name[100];
    // int adv_num = 0;
    // if(adv_name_len > 0)
    // {
    // 	for(; adv_num < adv_name_len; adv_num++)
    // 	{
    // 		sprintf(adv_name, "%s%s", adv_name, (char*)(tmp_adv_name+adv_num));
    // 	}
    // 	printf("adv_name = %s\n", adv_name);
    // 	cJSON_AddItemToObject(data, "bleName", cJSON_CreateString(adv_name));
    // }

    char address[30];

    // bzero(address, sizeof(address));
    // ble_array2str(tmp_sta_mac, address);
    sprintf(address, "%02X:%02X:%02X:%02X:%02X:%02X", MAC2STR(tmp_sta_mac));
    cJSON_AddItemToObject(data, "dev_wifi_sta_mac", cJSON_CreateString(address));

    sprintf(address, "%02X:%02X:%02X:%02X:%02X:%02X", MAC2STR(tmp_ble_mac));
    cJSON_AddItemToObject(data, "dev_ble_mac", cJSON_CreateString(address));

    // bzero(address, sizeof(address));
    // ble_array2str(scan_result->scan_rst.bda, address);
    sprintf(address, "%02X:%02X:%02X:%02X:%02X:%02X", MAC2STR(scan_result->scan_rst.bda));
    cJSON_AddItemToObject(data, "mac", cJSON_CreateString(address));

    cJSON_AddNumberToObject(data, "rssi", scan_result->scan_rst.rssi);

    char adv_data[62];
    memset(adv_data, 0, 62);
    int adv_num = 0;
    if (scan_result->scan_rst.adv_data_len > 0) {
        for (adv_num = 0; adv_num < scan_result->scan_rst.adv_data_len; adv_num++) {
            // sprintf(adv_data, "%s%s", adv_data, (char*)(scan_result->scan_rst.ble_adv[adv_num]));
            itoa_16(scan_result->scan_rst.ble_adv[adv_num], &adv_data[2 * adv_num]);
            // adv_data[adv_num] = (char)scan_result->scan_rst.ble_adv[adv_num];
        }
        adv_data[2 * scan_result->scan_rst.adv_data_len] = '\0';
        // memcpy(adv_data, scan_result->scan_rst.ble_adv, scan_result->scan_rst.adv_data_len);
        // printf("adv_data = %s\n", adv_data);
        cJSON_AddItemToObject(data, "adv_data", cJSON_CreateString(adv_data));
        cJSON_AddNumberToObject(data, "adv_data_len", (2 * scan_result->scan_rst.adv_data_len));
        need_send = true;
    }

    char adv_rsp_data[62];
    memset(adv_rsp_data, 0, 62);
    if (scan_result->scan_rst.scan_rsp_len > 0) {
        for (adv_num = 0; adv_num < scan_result->scan_rst.scan_rsp_len; adv_num++) {
            // sprintf(adv_rsp_data, "%s%s", adv_rsp_data, (char*)(scan_result->scan_rst.ble_adv[scan_result->scan_rst.adv_data_len+adv_num]));
            itoa_16(scan_result->scan_rst.ble_adv[scan_result->scan_rst.adv_data_len + adv_num], &adv_rsp_data[2 * adv_num]);
        }
        adv_rsp_data[2 * scan_result->scan_rst.scan_rsp_len] = '\0';
        // esp_log_buffer_hex("IOT DEMO", &scan_result->scan_rst.ble_adv[scan_result->scan_rst.adv_data_len], scan_result->scan_rst.scan_rsp_len);
        // printf("adv_rsp_data = %s\n", adv_rsp_data);
        cJSON_AddItemToObject(data, "scan_resp", cJSON_CreateString(adv_rsp_data));
        cJSON_AddNumberToObject(data, "scan_resp_len", (2 * scan_result->scan_rst.scan_rsp_len));
        need_send = true;
    }

    if (need_send) {
        char* out = NULL;
        out = cJSON_PrintUnformatted(data);
        // printf("%s\n", out);

        esp_mqtt_client_publish(mqtt_client, MQTT_ADV_TOPIC, out, strlen(out), 0, 0);

        free(out);
    }
    cJSON_Delete(data);
}
