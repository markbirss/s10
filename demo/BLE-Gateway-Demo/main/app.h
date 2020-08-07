/*****************************************************************************
 * @file  
 * @brief 
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

#ifndef _APP_H_
#define _APP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_bt.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gattc_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "mqtt_client.h"

#include "sdkconfig.h"

#include "esp_eth.h"
#include "rom/gpio.h"
#include "tcpip_adapter.h"
#include "driver/gpio.h"
#include "driver/periph_ctrl.h"
#include "esp_timer.h"
#include "driver/uart.h"

#include "esp_http_client.h"
#include "esp_ota_ops.h"

#include "cJSON.h"
#include "lwip/apps/sntp.h"
#include <sys/time.h>

#include "esp_efuse.h"
#include "esp_efuse_table.h"
#include "rom/efuse.h"

#include "esp_heap_caps.h"

#include "esp_partition.h"

#define TAG "GL-S10"
#define STORAGE_NAMESPACE           "storage"

/***** BLE Message *****/
#define bMagicNumber	0
#define nLength			1
#define nCmdId			3		// OP Code
#define BODY			4

/********** OP code **********/
//WIFI 0x01-0x64
#define SET_WIFI_CONFIG                 0x01
#define START_WIFI                      0x02
#define STOP_WIFI                       0x03
#define RESTART_WIFI                    0x04
#define GET_NETWORK_STATE               0x05
#define CLEAR_WIFI_CONFIG               0x06

//MQTT 0x65-0x96
#define SET_MQTT_URI                    0x65
#define START_MQTT                      0x66
#define STOP_MQTT                       0x67
#define RESTART_MQTT                    0x68
#define GET_MQTT_CONFIG                 0x69
#define CLEAR_MQTT_CONFIG               0x6A
#define MQTT_SUBSCRIBE                  0x6B
#define MQTT_UNSUBSCRIBE                0x6C
#define MQTT_PUBLISH                    0x6D
#define MQTT_SUBSCRIBE_MES              0X6E

//ota
#define GET_VERSION                     0xF0
#define SET_OTA_URL                     0xF1
#define OTA_STATUS						0xF2

//TEST 0xFA-0xFE
#define RESTART                         0xFA
#define GET_MAC                         0xFB
#define GET_SN                          0xFC
#define GET_DEVICE_ID                   0xFD


#define GPIO_INPUT_IO_BUTTON     33
#define GPIO_INPUT_PIN_SEL  (1ULL<<GPIO_INPUT_IO_BUTTON)
#define ESP_INTR_FLAG_DEFAULT 0

#define GPIO_OUTPUT_IO_LED1  14  //power
#define GPIO_OUTPUT_IO_LED2  12  //bluetooth
#define GPIO_OUTPUT_IO_LED3  32  //network
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_IO_LED1) | (1ULL<<GPIO_OUTPUT_IO_LED2) | (1ULL<<GPIO_OUTPUT_IO_LED3))

/* BLE command string*/
#define CMD_MAX_LEN         1024
/* ble and cloud cmd queue max length */
#define CMD_QUEUE_LEN        10

extern bool WIFI_SWITCH;

// extern char ble_command[CMD_MAX_LEN]; //ble_command[0] store the command length
// extern char notify_msg[CMD_MAX_LEN];//notify_msg[0] store the msg length
extern char network_ip[50];

extern int ble_connect;
extern bool mqtt_connect;
extern bool wifi_connect;
extern bool use_ethernet;
extern bool network_connect;
extern bool ethernet_connect;
extern int LED_flash_level;
extern bool normal_working_led;

extern wifi_config_t sta_config;

extern esp_mqtt_client_config_t mqtt_cfg;
extern esp_mqtt_client_handle_t mqtt_client;

extern EventGroupHandle_t ota_group;
extern int OTA_START_BIT;

extern EventGroupHandle_t network_group;
extern int NETWORK_CONNECTED_BIT;
extern int WIFI_CONNECTED_BIT;
extern char sta_mac[13];
extern char cloud_cmd_topic[50];
extern char response_topic[50];
extern char status_topic[50];
extern char adv_topic[50];
extern char online_topic[50];
extern char offline_topic[50];

//app.func.c
char *create_string(const char *ptr, int len);
void set_wifi_config(char* value, uint16_t len);
void restart_wifi(void);
void start_wifi(void);
void stop_wifi(void);
void clean_wifi_config(void);
void clean_mqtt_config(void);
void set_mqtt_uri(char* value,uint16_t len);
void start_mqtt(void);
void stop_mqtt(void);
void restart_mqtt(void);
void get_network_state(void);
void get_mqtt_config(void);
void subscribe_mqtt(char* value,uint16_t len);
void unsubscribe_mqtt(char* value,uint16_t len);
void publish_mqtt(char* value,uint16_t len);
void get_current_version(void);
void get_mac(void);
// void get_sn(void);
// void get_device_id(void);
void publish_scan_result(esp_ble_gap_cb_param_t* scan_result);
void send_OTA_status(int status);
void set_OTA_url(char* value,uint16_t len);

esp_err_t read_item_from_nvs(char* id, char* value);
bool read_wifi_config_from_nvs(void);
void write_item_to_nvs(char* id, char* value);
void write_wifi_config_to_nvs(void);
void write_mqtt_config_to_nvs(void);
bool read_mqtt_config_from_nvs(void);
int ble_array2str(const esp_bd_addr_t eui48, char* str);
void str2uint8_array(char* str, uint8_t* array, int len);
void mac_array2str(uint8_t* mac, char* str);
void ltoa(long num, char* str);
void itoa_16(uint8_t n,char* str);


//main.c
long get_time(void);
void init_time(void);
void initialize_sntp(void);
esp_err_t network_event_handler(void *ctx, system_event_t *event);

//ota_http.c
extern char* user_ota_url;
esp_err_t esp_http_ota(const esp_http_client_config_t *config);
void initialise_ota(void);
void ota_task(void * pvParameter);

//mqtt.c
void mqtt_app_start(void);
void mqtt_notify_pack(esp_mqtt_event_handle_t event);
esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event);

//ble.c
extern bool ble_server_connect;
void send_ble_notify();
uint8_t get_ble_command(uint8_t* value, uint16_t len);
void init_ble(void);
void create_return_notify(uint8_t cmdID, char* body, uint16_t bodyLen);
int create_ble_command(uint8_t cmdID, char *body);


//ethernet.c
void initialise_eth(void);


//led_button.c
void led_task(void * pvParameter);
void init_timer(void);
void initialise_ledTask(void);

//wifi.c
void wifi_init_sta();

//ble_execute.c
extern QueueHandle_t ble_cmd_queue;
void initialise_ble_executeTask(void);
//transpondTask.c
extern QueueHandle_t scan_result_queue;
void init_transpondTask(void);
void transpond_task(void * pvParameter);
void send2cloud(esp_ble_gap_cb_param_t* scan_result);

//ble_scan_task.c
void init_ble_scan_task(void);



#endif