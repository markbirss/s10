#!/bin/bash

git submodule deinit -f .
git submodule init
git submodule update

#准备IDF SDK
cd sdk/esp-idf/

git reset --hard bf022060964128556b3d3205b65c5d35df9beef6
# git am --abort
# git am ../def_config/esp-idf_parch/*.patch

# cp -rf ../../def_config/def_config_esp-idf/.gitmodules .
git submodule deinit -f .
git submodule init
git submodule update

./install.sh

cd ../../

# cp -rf ./def_config/BLE-Gateway-Demo_sh/rapid_flash.sh ./demo/BLE-Gateway-Demo/
# cd  demo/BLE-Gateway-Demo/

echo -e "config OK!!!"
