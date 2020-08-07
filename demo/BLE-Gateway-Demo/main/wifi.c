/*****************************************************************************
 * @file  mqtt.c
 * @brief Handle wifi related events  
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

/*Wifi station config*/
wifi_config_t sta_config;

EventGroupHandle_t network_group;
int NETWORK_CONNECTED_BIT = BIT0;
int WIFI_CONNECTED_BIT = BIT1;

void wifi_init_sta()
{
    ESP_ERROR_CHECK(esp_event_loop_init(network_event_handler, NULL) );
	network_group = xEventGroupCreate();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
	if(read_wifi_config_from_nvs())
	{
		ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_config) );
	}
    ESP_ERROR_CHECK(esp_wifi_start() );
	WIFI_SWITCH = true;
}


/************************* sntp ******************************/
time_t now;
struct tm timeinfo;

long get_time(void)
{
	setenv("TZ", "CST-8", 1);
    tzset();
    
	struct timeval now_time;
	gettimeofday(&now_time, NULL);
    // struct tm *tm_local = localtime(&now_time);

	// sprintf(str, "%d-%d-%dT%d:%d:%dZ", tm_local->tm_year+1900, tm_local->tm_mon+1, tm_local->tm_mday, tm_local->tm_hour, tm_local->tm_min, tm_local->tm_sec);
	return now_time.tv_sec;
}

void initialize_sntp(void)
{
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();
}

void init_time(void)
{
	xEventGroupWaitBits(network_group, NETWORK_CONNECTED_BIT,
                        false, true, portMAX_DELAY);
	initialize_sntp();

	// wait for time to be set
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 10;
    while(timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) 
	{
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        time(&now);
        localtime_r(&now, &timeinfo);
    }

	sntp_stop();
}
