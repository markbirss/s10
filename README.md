# Introduction

GL-S10 is a WiFi-BLE gateway using ESP32 as the master MCU. It can connect to superior WiFi network as STA and accept BLE command at the same time. It provides a network interface, a reset button and three LEDs.

# Directory Structure
```
─ app_configuration_tool  #Configuration tool of ble gateway
│   └── android
│       └── apk
├── demo
│   └── BLE-Gateway-Demo  #Source code of ble gateway demo
│
├── docs
│   ├── GL-S10 BLE-Gateway User Guide.pdf
│   └── GL-S10 Datasheet CN.pdf
└── VERSION_FILE          #Revision information record
```
# Prerequisites

本教程是基于ubuntu 20.10版本搭建的，建议使用该版本及以上版本搭建环境，要注意的是低于这个版本可能会存在兼容问题。

![image-20210419115603753](docs/001.png)

# Environment configuration

## Installing Packages

```
sudo apt-get update
sudo apt-get install git wget flex bison gperf python3 python3-pip python3-setuptools libffi-dev libssl-dev libncurses5-dev
sudo update-alternatives --install /usr/bin/python python /usr/bin/python3 10 && alias pip=pip3
```

![image-20210419150119645](docs/002.png)

## Downloading Source

```
cd ~
mkdir -p ~/esp
cd esp
git clone https://github.com/espressif/esp-idf.git
git clone https://github.com/gl-inet/s10.git
cd esp-idf
git reset --hard bf022060964128556b3d3205b65c5d35df9beef6     //Make sure you switch here commit-bf022060964128556b3d3205b65c5d35df9beef6
```

![image-20210419145339542](docs/003.png)

## Update the child library

```
git submodule deinit -f .
git submodule init
git submodule update
./install.sh
```

![image-20210419145718332](docs/004.png)

![image-20210419145639185](docs/005.png)

![image-20210419145618459](docs/006.png)

## Compile the project

```
cd ~/esp/s10/demo/BLE-Gateway-Demo/
. ~/esp/esp-idf/export.sh
make clean
make all
```

![image-20210419151121949](docs/007.png)

![image-20210419151720022](docs/008.png)

## Firmware writing

1.这里需要准备一个USB-TTL 这里我们用的是Ai-Thinker的USB-T1(CP2102) [CP2102驱动链接(**windos**)](https://docs.ai-thinker.com/_media/tools/serial_driver_windos.7z)

<img src="docs/009.png" alt="image-20210419152748217" style="zoom:50%;" />

<img src="docs/010.png" alt="image-20210419152858222" style="zoom:51%;" />

2.以下是我们GL_S10 PCB上的烧入引脚分布：

<img src="docs/011.png" alt="image-20210419154151250"  />

3.我们需要将GL-S10和PC通过USB-T1连接上：

![image-20210419160329072](docs/012.png)

4.查看USB-TTL否连接到Linux下同时赋予权限:

```
ll /dev/ttyUSB*
sudo chmod 777 /dev/ttyUSB0
```

![image-20210419164304842](docs/013.png)

5.当设备确认连接后就可以开始写入固件了：

```
make flash
```

![image-20210419164422002](docs/014.png)



## Serial port log listening

```
make monitor
```

![image-20210419164656440](docs/015.png)

## OTA

GL-S10的OTA是通过http访问服务器获取固件，相关操作步骤见[GL-S10 BLE-Gateway User Guide](docs/GL-S10 BLE-Gateway User Guide.pdf)



# How to use
If you want to know the s10 hardware parameters, please refer to the [GL-S10 Datasheet CN](docs/GL-S10 Datasheet CN.pdf)

If you want to know how to use the ble gateway, please refer to the [GL-S10 BLE-Gateway User Guide](docs/GL-S10 BLE-Gateway User Guide.pdf)
