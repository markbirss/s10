/*****************************************************************************
 * @file  ble_scan_task.c
 * @brief Main loop of ble scan task 
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


#define SCAN_TIME				1
#define SCAN_WAIT				10

static void ble_scan_task(void * pvParameter);

void init_ble_scan_task(void)
{
	xTaskCreate(ble_scan_task, "ble_sacnTask", 2048, NULL, 5, NULL);
}

static void ble_scan_task(void * pvParameter)
{
	while(1)
	{
		esp_ble_gap_start_scanning(1);

		printf(" esp_get_free_heap_size : %d  \n",esp_get_free_heap_size());

		vTaskDelay((SCAN_WAIT * 1000) / portTICK_PERIOD_MS);
	}
}