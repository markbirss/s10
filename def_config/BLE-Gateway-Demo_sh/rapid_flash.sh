#!/bin/sh

project_path=$(cd `dirname $0`; pwd)  #获取当前目录
project_name="${project_path##*/}"    #获取当前所在文件名称
echo -e ""
echo -e "当前文件路径：" $project_path

if [ $project_path != "/bin" ]; then

	#环境变量设置
	cd ../../
	sdk_path=$(cd `dirname $0`; pwd)
	export IDF_PATH=$sdk_path/sdk/esp-idf
	. $IDF_PATH/export.sh
	cd $project_path

	# idf.py set-target esp32
	# idf.py set-target esp32s2
	# idf.py set-target esp32s3
	# make clean

	#配置
	# idf.py menuconfig
	# make menuconfig

	#编译
	# idf.py build
	make -j99

	#擦写flash
	# idf.py -p /dev/ttyUSB0 -b 921600 erase_flash
	make erase_flash

	#烧入
	# idf.py -p /dev/ttyUSB0 -b 921600 flash
	make flash

	#串口监听
	#idf.py -p /dev/ttyUSB0 monitor
	make monitor

else
        echo -e ""
        echo -e ""
        echo -e "错误：没有正确获取到文件当前路径，请换一个终端工具试试！！！" 
        echo -e ""
        echo -e ""
fi
