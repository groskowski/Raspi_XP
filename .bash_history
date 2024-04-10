ls
sudo raspi-config
ifconfig
sudo raspi-config
ifconfig
sudo reboot
ls
ifconfig
sudo raspi-config
ifconfig
ping google.com
reboot
ping google.com
sudo raspi-config
ping google.com
ifconfig
sudo raspi-config
ifconfig
nmcli status
sudo cat /etc/wpa_supplicant/wpa_supplicant.conf
nmcli nmcli device wifi connect "myssid" password "mypassword"
nmcli device wifi connect "myssid" password "mypassword"
nmcli device wifi scan
sudo nmcli device wifi rescan
sudo nmcli device wifi list
nmcli device wifi connect "myssid" password "mypassword"
iw
iw list
nmcli device wifi connect "myssid" password "mypassword" wpa3-enterprise
nmcli device wifi add myssid password "mypassword" wpa3-enterprise
nmcli device wifi connect myssid password "mypassword" wpa3-enterprise
nmcli device wifi connect myssid password "mypassword"
sudo nmcli device wifi list
sudo nmcli device wifi rescan
sudo nmcli device wifi list
nmcli connection delete myssid
nmcli device wifi connect myssid password mypassword
sudo nmcli device wifi rescan
sudo nmcli device wifi list
sudo nmcli device wifi rescan
sudo nmcli device wifi list
nmcli device wifi connect myssid password mypassword
nmcli connection edit type wifi con-name myssid
ifconfig
sudo nmcli device wifi list
nmcli connection up myssid
ip addr show
sudo systemctl restart NetworkManager
ip addr show
sudo ip link set wlan0 down
sudo ip link set wlan0 up
ip addr show
ifconfig
sudo iwlist wlan0 scan | grep ESSID
sudo cat etc/wpa_supplicant/wpa_supplicant.conf
sudo cat /etc/wpa_supplicant/wpa_supplicant.conf
sudo nano /etc/wpa_supplicant/wpa_supplicant.conf
sudo dmesg | grep -i wlan
sudo wpa_supplicant -B -i wlan0 -c wpa_supplicant.conf
sudo wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant/wpa_supplicant.conf
sudo killall wpa_supplicant
sudo wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant/wpa_supplicant.conf
ifconfig
cat /var/run/wpa_supplicant
ls /var/run/wpa_supplicant
ls /var/run/wpa_supplicant wlan0
ls /var/run/wpa_supplicant/wlan0
cat /var/run/wpa_supplicant/wlan0
sudo cat /var/run/wpa_supplicant/wlan0
sudo ls /var/run/wpa_supplicant/wlan0
sudo killall wpa_supplicant
ps aux | grep wpa_supplicant
ps aux
ps aux | grep wpa_supplicant
sudo rm /var/run/wpa_suuplicant/*
sudo rm /var/run/wpa_supplicant/*
sudo wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant/wpa_supplicant.conf
sudo rm /var/run/wpa_supplicant/*
sudo killall wpa_supplicant
sudo systemctl restart NetworkManager
sudo cat /etc/wpa_supplicant/wpa_supplicant.conf
sudo nano systemctl restart NetworkManager
sudo nano /etc/wpa_supplicant/wpa_supplicant.conf
sudo killall wpa_supplicant
sudo rm /var/run/wpa_supplicant/*
sudo systemctl restart NetworkManager
sudo wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant/wpa_supplicant.conf
sudo rm /var/run/wpa_supplicant/wlan
sudo rm /var/run/wpa_supplicant/wlan0
sudo nano /etc/wpa_supplicant/wpa_supplicant.conf
sudo wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant/wpa_supplicant.conf
sudo killall wpa_supplicant
sudo wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant/wpa_supplicant.conf
sudo rm /var/run/wpa_supplicant/wlan0
sudo wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant/wpa_supplicant.conf
sudo nmcli device disconnet wlan0
sudo nmcli device disconnect wlan0
sudo wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant/wpa_supplicant.conf
sudo nmcli device disconnect wlan0
sudo rm /var/run/wpa_supplicant/wlan0
sudo wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant/wpa_supplicant.conf
sudo rm /var/run/wpa_supplicant/wlan0
sudo nmcli device disconnect wlan0

sudo dmesg | grep -i wlan
sudo dmesg | grep -i wlan0
sudo dmesg | grep wlan0
sudo dmesg | grep 
sudo dmesg | grep net
sudo dmesg | grep wlan
sudo dmesg
sudo dmesg | grep err
sudo nmcli device disconnect wlan0
sudo rm /var/run/wpa_supplicant/wlan0
sudo nano /etc/wpa_supplicant/wpa_supplicant.conf
sudo modprobe -r brcmfmac
sudo modprobe brcmfmac
sudo wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant/wpa_supplicant.conf
sudo rm /var/run/wpa_supplicant/wlan0
sudo killall wpa_supplicant
sudo dmesg | grep -i wlan0
sudo nmcli device disconnect wlan0
sudo wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant/wpa_supplicant.conf
wlan0
clear
sudo wpa_cli
sudo killall wpa_supplicant
nano wpa_sup.conf
sudo wpa_supplicant -B -i wlan0 -c wpa_sup.conf
reboot
ls
cd ..
ls
cd pi
git
git init
sudo apt-get install git
ls
git init
ls
git
git status
git add *
git commit -m "initial_commit"
git status
git commit -m "initial_commit"
git config --global user.email 
git config --global user.email "rpi@gmail.com"
git config --global user.name "rpi"
ls
git commit -m "initial_commit"
ls
clear
git status
git add *
git commit -m "Initial commit 2"
ls
clear
sudo apt install mariadb-server
sudo apt update
sudo mysql_secure_installation
sudo mariadb
sudo mysqladmin -u admin -p version
clear
sudo mariadb
cd code
python
python ExampleMariaDBInsertion.py
pip 
python -m ensurepip --upgrade
python get-pip.py
pip install pip
sudo apt install python3-pip
pip -v
clear
pip install mysql
clear
pip install mysql-connector-python
python ExampleMariaDBInsertion.py
sudo apt update --all
sudo apt update -all
sudo apt update -a
sudo apt update-all
sudo apt update
sudo apt upgrade
python ExampleMariaDBInsertion.py
sudo mariadb
ss -tulwn
clear
nmcli connection show
nmcli
nmcli -h
nmcli c
nmcli c -h 
nmcli connection show
sudo cat /etc/wpa_supplicant/wpa_supplicant.conf
nmcli cnmcli device
nmcli device
nmcli device wifi connect myssid password mypassword
nmcli device list
nmcli device wifi  list
nmcli device wifi rescan
sudo nmcli device wifi rescan
nmcli device wifi rescan
nmcli device wifi  list
nmcli device wifi  list | awk '$7 >= 70'
nmcli device wifi rescan
sudo nmcli device wifi rescan
nmcli device wifi  list | awk '$7 >= 70'
nmcli device wifi connect myssid password mypassword
sudo nmcli device wifi connect myssid password mypassword
sudo wpa_supplicant -B -i wlan0 -c wpa_sup.conf
ifconfig
sudo ifconfig wlan0 down
sudo wpa_supplicant -B -i wlan0 -c wpa_sup.conf
sudo ifconfig wlan0 up
ifconfig
ifconfig wlan0
ifconfig
nmcli devices
nmcli device
nmcli device wifi rescan
nmcli device wifi list
nmcli device wifi rescan
nmcli device wifi list
sudo wpa_supplicant -B -i wlan0 -c wpa_sup.conf
nmcli device wifi list
sudo ifconfig wlan0 down
sudo ifconfig wlan0 up
ifconfig
sudo ifconfig wlan0 down
ifconfig
sudo ifconfig wlan0 up
ifconfig
ifconfig wlan0
ifconfig
ifconfig wlan0
nmcli device wifi list
ifconfig wlan0
sudo nmcli device wifi connect myssid password mypassword
clear
reboot
sudo apt-get install git wget flex bison gperf python3 python3-pip python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0
cd code
mkdir -p ~/esp
cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git
cd ~/esp/esp-idf
./install.sh esp32
cd ~/esp
cp -r $IDF_PATH/examples/wifi/softap_sta .
cd ~/esp/softap_sta
cd ~/esp/softap_sta. $HOME/esp/esp-idf/export.sh
. $HOME/esp/esp-idf/export.sh
cd ~/esp
cp -r $IDF_PATH/examples/wifi/softap_sta .
cd ~/esp/softap_sta
idf.py set-target esp32
idf.py menuconfig
idf.py build
idf.py -p PORT flash monitor
lsusb
ls /dev
idf.py -p /dev/tty0 flash monitor
idf.py -p /dev/tty0/ flash monitor
idf.py -p /dev/tty1/ flash monitor
lsusb
sudo idf.py -p /dev/tty1/ flash monitor
idf.py -p /dev/tty1/ flash monitor
idf.py -p dev/tty0/ flash monitor
idf.py -p /dev/ttyUSB0/ flash monitor
idf.py -p /dev/ttyUSB1/ flash monitor
idf.py -p /dev/ttyUSB0 flash monitor
ifconfig
nmcli device wifi list
nmcli device connect
nmcli device connect wifi
ls
ipconfig
ifconfig
nmcli con down 
nmcli con list
nmcli con down IIT-IoT
nmcli device wifi connect ESP32_AP --ask
ifconfig
cd code
ls
nano TestServer.py
nmcli con down ESP32_AP
nmcli device wifi connect IIT_IoT --ask
nmcli con list
nmcli con up IIT-IoT
nmcli con down IIT-IoT
nmcli device wifi connect IIT_IoT --ask
nmcli device wifi connect ESP32_AP --ask
ifconfig
python TestServer.py
ifconfig
nmcli con down ESP32_AP
nmcli con up IIT-IoT
nmcli con down ESP32_AP
nmcli con down IIT-IoT
nmcli device wifi connect ESP32_AP --ask
ifconfig
nmcli device wifi list
python TestServer.py
reboot
. $HOME/esp/esp-idf/export.sh
cd ~/esp/softap_sta
idf.py set-target esp32
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
s
cd ..
cd code
python TestServer.py
ping localhost:8000
ifconfig
clear
sudo ufw allow 8000
sudo apt install ufw
sudo ufw allow 8000
ifconfig
cd code
sudo ufw allow 8000
python TestServer.py
nmcli device wifi connect ESP32_AP --ask
nmcli device wlan0 down ESP32_AP --ask
nmcli device wifi ESP32_AP down
nmcli device ESP32_AP down
nmcli device
nmcli device down
nmcli con down ESP32_AP
nmcli con up IIT_IoT
nmcli device wifi IIT_IoT
nmcli con up IIT-IoT
python TestServer.py
ifconfig
python TestServer.py
clerar
clear
alskd jf lsadkfj
reboot
cd esp
cd softap_sta
idf.py build
. $HOME/esp/esp-idf/export.sh
idf.py build
clear
idf.py build
idf.py set-target esp32
idf.py build
. $HOME/esp/esp-idf/export.sh
cd ~/esp
cp -r $IDF_PATH/examples/wifi/softap_sta .
cd softap_sta
idf.py set-target esp32
idf.py build
cd softap
cd softAP
ls
idf.py build
idf.py fullclean
idf.py build
idf.py -p PORT flash monitor
idf.py -p /dev/ttyUSB0 flash monitor
cd code
python TestServer.py
cd esp
cd //
cd ..
ls
cd home
ls
cd pi
ls
cd esp
cd softap_sta
ls
idf.py build
. $HOME/esp/esp-idf/export.sh
idf.py set-target esp32
idf.py build
idf.py -p PORT flash monitor
idf.py -p /dev/ttyUSB0 flash monitor
