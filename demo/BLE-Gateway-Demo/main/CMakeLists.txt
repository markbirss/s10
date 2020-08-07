 ###################################################################
 # Copyright 2020 GL-iNet. https://www.gl-inet.com/
 # 
 # Licensed under the Apache License, Version 2.0 (the "License");
 # you may not use this file except in compliance with the License.
 # You may obtain a copy of the License at
 #
 # http://www.apache.org/licenses/LICENSE-2.0
 # 
 # Unless required by applicable law or agreed to in writing, software
 # distributed under the License is distributed on an "AS IS" BASIS,
 # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 # See the License for the specific language governing permissions and
 # limitations under the License.
 ####################################################################
 
set(COMPONENT_SRCS  "main.c"
					"ota_http.c"
					"app.func.c"
					"ble.c"
					"ethernet.c"
					"led_button.c"
					"wifi.c"
					"mqtt.c"
					"transpondTask.c"
					"ble_scan_task.c"
					"ble_execute.c")

set(COMPONENT_ADD_INCLUDEDIRS ".")

register_component()
