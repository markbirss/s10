/*****************************************************************************
 * @file  app_func.c
 * @brief provide function of applicatuon
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

char *create_string(const char *ptr, int len)
{
    char *ret;
    if (len <= 0) {
        return NULL;
    }
    ret = calloc(1, len + 1);
    //ESP_MEM_CHECK(TAG, ret, return NULL);
    memcpy(ret, ptr, len);
    return ret;
}

void set_wifi_config(char* value, uint16_t len)
{
	esp_wifi_disconnect();

    uint8_t res = 0;
	int num = 0;
	char tmp_ssid[33];
	char tmp_password[64];
	printf("%s len:%d \n", (char*)value, len);
	while(num < len) // partition string
	{
		if(value[num] == '@')
		{
			break;
		}
		tmp_ssid[num] = value[num];
		num++;
	}
	
	// set wifi config
    strncpy((char*)sta_config.sta.ssid,tmp_ssid,strlen(tmp_ssid));
    sta_config.sta.ssid[num] = '\0';
	printf("ssid: %s\n", sta_config.sta.ssid);

	int password_num = len - num - 1;
	printf("ssid_num: %d\n", num);
	printf("password_num: %d\n", password_num);
	if(password_num == 0)
	{
		memset(sta_config.sta.password, 0, 64);
	}else{
		strcpy(tmp_password, &value[num+1]);
		strncpy((char*)sta_config.sta.password,tmp_password,strlen(tmp_password));
		sta_config.sta.password[password_num] = '\0';
		printf("password: %s\n", sta_config.sta.password);
	}
    
	if(esp_wifi_set_config(WIFI_IF_STA, &sta_config))
    {
        res = 0x01;
		create_return_notify(SET_WIFI_CONFIG, (char*)&res, 1);
		return;
    }

	if(esp_wifi_connect())
	{
        res = 0x01;
		create_return_notify(SET_WIFI_CONFIG, (char*)&res, 1);
		return;
	}

	EventBits_t uxBits;
	TickType_t xTicksToWait = 20000 / portTICK_PERIOD_MS;
	uxBits = xEventGroupWaitBits(network_group, WIFI_CONNECTED_BIT,
                        false, true, xTicksToWait);
	if(( uxBits & WIFI_CONNECTED_BIT ) != 0 )
	{
		res = 0x00;
		create_return_notify(SET_WIFI_CONFIG, (char*)&res, 1);
	}else{
		res = 0x01;
		create_return_notify(SET_WIFI_CONFIG, (char*)&res, 1);
	}
    
}

void restart_wifi(void)
{
    uint8_t res = 0;
    esp_wifi_disconnect();
    ESP_LOGI(TAG, "Connect to AP SSID: %s, password: %s\n",sta_config.sta.ssid,sta_config.sta.password);
    if(esp_wifi_connect())
    {
        res = 0x01;
    }
    create_return_notify(RESTART_WIFI,(char*)&res, 1);
}

void start_wifi(void)
{
    uint8_t res = 0;
    ESP_LOGI(TAG, "Connect to AP SSID: %s, password: %s\n",sta_config.sta.ssid,sta_config.sta.password);
    if(esp_wifi_connect())
    {
        res = 0x01;
    }else{
		WIFI_SWITCH = true;
	}
    create_return_notify(START_WIFI,(char*)&res, 1);
}

void stop_wifi(void)
{
    uint8_t res = 0;
    if(esp_wifi_disconnect())
	{
		res = 0x01;
	}else{
		WIFI_SWITCH = false;
		wifi_connect = false;
	}
    ESP_LOGI(TAG, "stop wifi\n");
    create_return_notify(STOP_WIFI,(char*)&res, 1);
}

void clean_wifi_config(void)
{
	memset(sta_config.sta.ssid, 0, sizeof(sta_config.sta.ssid));
	memset(sta_config.sta.password, 0, sizeof(sta_config.sta.password));
	strncpy((char*)sta_config.sta.ssid,"0",strlen("0"));
	strncpy((char*)sta_config.sta.password,"0",strlen("0"));
	write_wifi_config_to_nvs();
	uint8_t res = 0;
	create_return_notify(CLEAR_WIFI_CONFIG, (char*)&res, 1);
}

void clean_mqtt_config(void)
{
	if(mqtt_cfg.uri)
	{
		free(mqtt_cfg.uri);
		mqtt_cfg.uri = NULL;
	}

	mqtt_cfg.uri = create_string("mqtt://username:password@127.0.0.1:61613", strlen("mqtt://username:password@127.0.0.1:61613"));
	write_mqtt_config_to_nvs();
	uint8_t res = 0;
	create_return_notify(CLEAR_MQTT_CONFIG, (char*)&res, 1);
}

void set_mqtt_uri(char* value,uint16_t len)
{
	uint8_t res = 0;
	if((!value)||(len == 0))
	{
		printf("error uri!\n");
		res = 1;
		create_return_notify(SET_MQTT_URI, (char*)&res, 1);
		return ;
	}
	// if(mqtt_cfg.uri)
	// {
	// 	free(mqtt_cfg.uri);
	// 	mqtt_cfg.uri = NULL;
	// }
	esp_mqtt_client_destroy(mqtt_client);

	char tmp[256];
	strcpy(tmp, value);
	printf("tmp uri: %s\n", tmp);
    mqtt_cfg.uri = create_string(tmp,len);
	printf("mqtt uri: %s\n", mqtt_cfg.uri);

	esp_err_t ret;
	
	mqtt_cfg.event_handle = mqtt_event_handler;
	mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
	// ret = esp_mqtt_client_set_uri(mqtt_client, tmp);
	if(mqtt_client)
    {
        res = 0x00;
		esp_mqtt_client_start(mqtt_client);
    }else{
		res = 0x01;
	}
	
    create_return_notify(SET_MQTT_URI, (char*)&res, 1);
}

void start_mqtt(void)
{
	esp_err_t ret;
	uint8_t res = 0;
	ret = esp_mqtt_client_start(mqtt_client);
	if(ret != ESP_OK)
	{
		res = 0x01;
		create_return_notify(START_MQTT,(char*)&res, 1);
		return ;
	}
	mqtt_connect = true;
	create_return_notify(START_MQTT,(char*)&res, 1);
}

void stop_mqtt(void)
{
	esp_err_t ret;
	uint8_t res = 0;
	ret = esp_mqtt_client_stop(mqtt_client);
	if(ret != ESP_OK)
	{
		res = 0x01;
		create_return_notify(STOP_MQTT,(char*)&res, 1);
		return ;
	}
	mqtt_connect = false;
	create_return_notify(STOP_MQTT,(char*)&res, 1);
}

void restart_mqtt(void)
{
    uint8_t res = 0;
    esp_mqtt_client_stop(mqtt_client);
    if(esp_mqtt_client_start(mqtt_client))
    {
        res = 0x01;
    }
    create_return_notify(RESTART_MQTT,(char*)&res, 1);
}

void get_network_state(void)
{
	char data[1024];
	memset(data, 0, 1024);
	data[0] = '0';
	if(network_connect)
	{
		data[1] = '1';
		data[2] = '@';
		strcpy(&data[5], network_ip); // get current ip
	}else{
		data[1] = '0';
		data[2] = '@';
	}

	if(use_ethernet)
	{
		data[3] = '1';
		data[4] = '@';
	}else{
		data[3] = '0';
		data[4] = '@';
	}

	if((network_connect) && (!use_ethernet))
	{
		data[strlen(data)] = '@';

		wifi_config_t wifi_cfg;
		if (esp_wifi_get_config(ESP_IF_WIFI_STA, &wifi_cfg) == ESP_OK) 
		{
			strcpy(&data[strlen(data)], (char*)wifi_cfg.sta.ssid);
		}
	}
	uint16_t dataLen = strlen(data);
	data[0] = (char)0x00;
	create_return_notify(GET_NETWORK_STATE, data, dataLen);
}

void get_mqtt_config(void)
{
	char data[512];
	memset(data, 0, 512);
	data[0] = '0';
	if(mqtt_connect)
	{
		data[1] = '1';
		data[2] = '@';
	}else
	{
		data[1] = '0';
		data[2] = '@';
	}
	char tmp[512];
	if(mqtt_cfg.uri)
	{
		strcpy(tmp, mqtt_cfg.uri);
		printf("tmp uri = %s\n", tmp);
		//cut off password
		int start_num = 0;
		int end_num = 0;
		int point = 7;
		while(point < strlen(tmp))
		{
			if(tmp[point] == ':')
			{
				start_num = point;
				//printf("start_num %d\n", start_num);
			}else if(tmp[point] == '@'){
				end_num = point;
				//printf("end_num %d\n", end_num);
				break;
			}
			point++;
		}
		strncpy(&data[3], tmp, start_num);
		strcpy(&data[strlen(data)], &tmp[end_num]);
	}else{
		strcpy(tmp, "NULL");
		printf("tmp uri: %s\n", tmp);
	}

	uint16_t dataLen = strlen(data);
	data[0] = (char)0x00;
	int num = 0;
	while(num < dataLen)
	{
		printf("%x ", (int)data[num]);
		num++;
	}
	printf("\n");

	create_return_notify(GET_MQTT_CONFIG, data, dataLen);
}

void subscribe_mqtt(char* value,uint16_t len)
{
    uint8_t res = 0;
    if(value[1] != '@')
        return;
    int qos = value[0] - 48;

	// value[len] = '\0';
	char topic[50];
	memset(topic,0,50);
	memcpy(topic, &value[2], len-2);
	topic[len-2] = '\0';
	printf("topic: %s\n", topic);
    if(esp_mqtt_client_subscribe(mqtt_client, topic, qos) < 0 )
    {
        res = 0x01;
    }
    create_return_notify(MQTT_SUBSCRIBE, (char*)&res, 1);
}

void unsubscribe_mqtt(char* value,uint16_t len)
{
    uint8_t res = 0;
    if(esp_mqtt_client_unsubscribe(mqtt_client, value) < 0)
    {
        res = 0x01;
    }
    create_return_notify(MQTT_UNSUBSCRIBE, (char*)&res, 1);
}

/*publish cmd format: qos@topic@data */
void publish_mqtt(char* value,uint16_t len)
{
    uint8_t res = 0;
    if(value[1] != '@')
        return;
    int qos = value[0] - 48;
    char* tmp = &value[2];
    char* topic = tmp;
    char* data = NULL;
    int tmp_len = 2;
    while(tmp)
    {
        tmp_len ++;
        if(*tmp == '@')
        {
            *tmp = 0;
            break;
        }
        tmp++;
    }
    data = ++tmp;
    if(esp_mqtt_client_publish(mqtt_client, topic, data, len - tmp_len, qos, 0) < 0)
    {
        res = 0x01;
    }
    create_return_notify(MQTT_PUBLISH, (char*)&res, 1);
}

void get_current_version(void)
{
	const esp_partition_t *running = esp_ota_get_running_partition();
	esp_app_desc_t running_app_info;
	char data[100];
	data[0] = '0';
	uint16_t dataLen = 0;
	if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) 
	{
		ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
		strcpy(&data[1], running_app_info.version);
		dataLen = strlen(data);
		data[0] = (char)0x00;
		create_return_notify(GET_VERSION, data, dataLen);
	}
	
}

void get_mac(void)
{
	uint8_t res = 0;
	uint8_t tmp_sta_mac[6];
	res = esp_wifi_get_mac(ESP_IF_WIFI_STA, tmp_sta_mac);
	if(res != ESP_OK)
	{
		create_return_notify(GET_MAC, (char*)&res, 1);
		return ;
	}

	int num;
	for(num = 0; num < 6; num++)
	{
		printf("mac[%d]: %d\n", num, tmp_sta_mac[num]);
	}
	char resp[7];
	resp[0] = 0x00;
	memcpy(&resp[1], tmp_sta_mac, 6);
	create_return_notify(GET_MAC, resp, 7);
}

/* use for product test
void get_sn(void)
{
	uint8_t res = 0;
	char sn[8];
	res = esp_efuse_read_field_blob(ESP_EFUSE_SN_64, sn, 64);
	if(res == ESP_OK)
	{
		char data[9];
		data[0] = 0x00;
		memcpy(&data[1], sn, 8);
		
		create_return_notify(GET_SN, data, 9);
	}else{
		create_return_notify(GET_SN, (char*)&res, 1);
	}
}

void get_device_id(void)
{
	uint8_t res = 0;
	char device_id[2];
	res = esp_efuse_read_field_blob(ESP_EFUSE_DEVICE_ID_HEAD, device_id, 16);
	if(res == ESP_OK)
	{
		char data[9];
		data[0] = 0x00;
		memcpy(&data[1], device_id, 2);
		uint8_t tmp_sta_mac[6];
		char str_mac[12];
		esp_wifi_get_mac(ESP_IF_WIFI_STA, tmp_sta_mac);
		mac_array2str(tmp_sta_mac, str_mac);
		memcpy(&data[3], &str_mac[7], 5);
		
		create_return_notify(GET_DEVICE_ID, data, 8);
	}else{
		create_return_notify(GET_DEVICE_ID, (char*)&res, 1);
	}

}
*/

void mac_array2str(uint8_t* mac, char* str)
{
	sprintf(str, "%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return ;
}

void set_OTA_url(char* value,uint16_t len)
{
	uint8_t res = 0;
	if(!value || (len <= 0) || (value[1] != '@'))
	{
		res = 1;
		create_return_notify(SET_OTA_URL, (char*)&res, 1);
		return ;
	}

	bool update_now = false;
	if(value[0] == '0')
	{
		update_now = true;
	}
	
	if(len > 2)
	{
		if(user_ota_url)
		{
			free(user_ota_url);
			user_ota_url = NULL;
		}

		user_ota_url = (char*)malloc(len-1);
		memcpy(user_ota_url, &value[2], len-2);
		user_ota_url[len-2] = '\0';
		printf("url: %s\n", user_ota_url);
	}

	if(update_now)
	{
		normal_working_led = false;
		LED_flash_level = 3;
		xEventGroupSetBits(ota_group, OTA_START_BIT);
	}
	create_return_notify(SET_OTA_URL, (char*)&res, 1);
}

void send_OTA_status(int status)
{
	if(!ble_server_connect)
	{
		return;
	}
	create_return_notify(OTA_STATUS, (char*)&status, 1);
	send_ble_notify();
}





esp_err_t read_item_from_nvs(char* id, char* value)
{
    nvs_handle my_handle;
    esp_err_t err;

    // Open
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return err;

    // Read
	size_t required_size = 0; 
    err = nvs_get_str(my_handle, id, NULL, &required_size);
	if (required_size == 0) {
        printf("Nothing saved yet!\n");
    } else {
        err = nvs_get_str(my_handle, id, value, &required_size);
        if (err != ESP_OK) {
            return err;
        }
        printf("read nvs----%s----%s\n", id, value);
    }

    // Close
    nvs_close(my_handle);
    return err;
}

bool read_wifi_config_from_nvs(void)
{
	esp_err_t ret;
	bool sta_config_read;
	char wifi_ssid[32];
	char wifi_password[64];
	ret = read_item_from_nvs("wifi_ssid", wifi_ssid);
	if(ret == ESP_OK)
	{
		sta_config_read = true;
		strncpy((char*)sta_config.sta.ssid, wifi_ssid, strlen(wifi_ssid));
    	sta_config.sta.ssid[strlen(wifi_ssid)] = '\0';
	}else
	{
		sta_config_read = false;
	}

	ret = read_item_from_nvs("wifi_password", wifi_password);
	if(ret == ESP_OK)
	{
		strncpy((char*)sta_config.sta.password, wifi_password, strlen(wifi_password));
    	sta_config.sta.password[strlen(wifi_password)] = '\0';
	}else
	{
		sta_config_read = false;
	}

	return sta_config_read;
}

void write_item_to_nvs(char* id, char* value)
{
	esp_err_t err;
	nvs_handle my_handle;
	err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
	
	if (err != ESP_OK) 
	{
		printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
	}   
	else {    
		nvs_set_str(my_handle, id, value);
		
		if (err != ESP_OK) 
		printf("write to nvs fail 0x%x\n",err);
		else
		{   
			printf("write the %s %s to nvs success\n",id,value);
			nvs_commit(my_handle);
		}   
		nvs_close(my_handle);
	}
}

void write_wifi_config_to_nvs(void)
{
	printf("save wifi config!\n");
	printf("ssid--------%s\n", (char*)sta_config.sta.ssid);
	printf("password----%s\n", (char*)sta_config.sta.password);
	write_item_to_nvs("wifi_ssid", (char*)sta_config.sta.ssid);
	write_item_to_nvs("wifi_password", (char*)sta_config.sta.password);
}

void write_mqtt_config_to_nvs(void)
{
	printf("save mqtt config!\n");
	if(mqtt_cfg.uri)//save uri
	{
		write_item_to_nvs("mqtt_uri", mqtt_cfg.uri);
	}else{
		printf("miss mqtt_uri! save mqtt config error!!\n");
	}
}

bool read_mqtt_config_from_nvs(void)
{
    nvs_handle my_handle;
    esp_err_t err;
	bool read_mqtt_config;

    // Open
    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return err;

    // Read
	size_t required_size = 0; 
    err = nvs_get_str(my_handle, "mqtt_uri", NULL, &required_size);
	if (required_size == 0) {
        printf("Nothing saved yet!\n");
		read_mqtt_config = false;
    } else {
		mqtt_cfg.uri = malloc(required_size);
        err = nvs_get_str(my_handle, "mqtt_uri", mqtt_cfg.uri, &required_size);
        if (err != ESP_OK) {
            read_mqtt_config = false;
        }
        printf("read nvs----mqtt_uri----%s\n", mqtt_cfg.uri);
		read_mqtt_config = true;
    }

    // Close
    nvs_close(my_handle);
    return read_mqtt_config;
}
void ltoa(long num, char* str)
{
	if((num < 0) || (str == NULL))
	{
 		return ;
	}

	char tmp[65];
	memset(tmp, 0, 65);
	int i, j = 0;
	do{
		i = num % 10;
		tmp[j] = i +'0';
		j++;
	}while((num /= 10) > 0);

	int len = strlen(tmp);	
	int k = 0;
	while(k < len)
	{
		str[k] = tmp[len-k-1];
		k++;
	}

	str[k] = '\0';
	// printf("ltoa: %s\n", str);
    return ;
}

void itoa_16(uint8_t n,char* str)
{
	int i,j,sign;
	char s[32];
	memset(s, 0, 32);

	if(n < 16)
	{
		str[0] = '0';
		if(n > 9)
		{
			str[1] = (n-10) + 'a';
		}else{
			str[1] = n + '0';
		}
	}
		
	int tmp1 = n % 16;
	if(tmp1 > 9)
	{
		str[1] = (tmp1-10) + 'a';
	}else{
		str[1] = tmp1 + '0';
	}

	int tmp2 = (n / 16) % 16;
	if(tmp2 > 9)
	{
		str[0] = (tmp2-10) + 'a';
	}else{
		str[0] = tmp2 + '0';
	}
}

void str2mac_array(char* str, uint8_t* mac)
{
	int i, j; 
    int num = 0;
	uint8_t tmp;
    char* p = str;
    for(i = 0; i < 6; i++)
    {
        tmp = 0;
        for(j = 0; j < 2; j++)
        {
            if((*p <= 0x39)&&(*p >= 0x30))
            {
                tmp = tmp*16 + *p - '0';
                p++;
            }
            else if((*p <= 0x46)&&(*p >= 0x41))
            {
                tmp = tmp*16 + *p - 'A' + 10;
                p++;
            }
            else if((*p <= 0x66)&&(*p >= 0x61))
            {
                tmp = tmp*16 + *p - 'a' +10;
                p++;
            }
            else
            {
                printf("ERROR: parameter error!\n");
                return;
            }
        }
		*(mac+i) = tmp;
		// printf("array[%d]: %d\n", i, mac[i]);
	}

	return;
}

int ble_array2str(const esp_bd_addr_t eui48, char* str)
{
    sprintf(str,"%02x%02x%02x%02x%02x%02x", \
				eui48[0], eui48[1], eui48[2],\
				eui48[3], eui48[4], eui48[5] );
    return 0;
}

void str2uint8_array(char* str, uint8_t* array, int len)
{
	int i, j; 
    int num = 0;
	uint8_t tmp;
    char* p = str;
    for(i = 0; i < len; i++)
    {
        tmp = 0;
        for(j = 0; j < 2; j++)
        {
            if((*p <= 0x39)&&(*p >= 0x30))
            {
                tmp = tmp*16 + *p - '0';
                p++;
            }
            else if((*p <= 0x46)&&(*p >= 0x41))
            {
                tmp = tmp*16 + *p - 'A' + 10;
                p++;
            }
            else if((*p <= 0x66)&&(*p >= 0x61))
            {
                tmp = tmp*16 + *p - 'a' +10;
                p++;
            }
            else
            {
                printf("ERROR: parameter error!\n");
                return;
            }
        }
		*(array+i) = tmp;
	}

	return;
}
