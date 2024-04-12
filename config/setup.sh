#!/bin/bash

echo "The setup protocol has started. Contact Gabriel Roskowski in case of malfunction."

# Check if nmcli is available
if ! command -v nmcli &> /dev/null; then
    echo "Installing Network Manager..."
    sudo apt install -y network-manager
fi

# Function to connect to Wi-Fi
connect_to_wifi() {
    read -p "Enter the Wi-Fi network name (SSID): " wifi_ssid
    read -s -p "Enter the Wi-Fi password: " wifi_password
    echo ""
    sudo nmcli dev wifi connect "$wifi_ssid" password "$wifi_password"
}

# Loop until connected to the internet
while true; do
    echo "Checking Internet connectivity..."
    if ping -c 2 google.com &> /dev/null; then
        echo "Internet connection is available."
        break
    else
        echo "Internet connection failed. Please enter your Wi-Fi settings."
    fi

    connect_to_wifi
done

# Restart DHCP client service if needed
if ping -c 2 google.com &> /dev/null; then
    echo ""
else
    sudo systemctl restart dhcpcd
fi


# Update package list and install required packages
sudo apt update
sudo apt install -y git python3 python3-pip php ufw mariadb-server wget

# Install pip packages
sudo pip3 install certifi==2020.6.20
sudo pip3 install chardet==4.0.0
sudo pip3 install colorzero==1.1
sudo pip3 install distro==1.5.0
sudo pip3 install gpiozero==1.6.2
sudo pip3 install idna==2.10
sudo pip3 install mysql-connector-python==8.3.0
sudo pip3 install numpy==1.19.5
sudo pip3 install picamera2==0.3.12
sudo pip3 install pidng==4.0.9
sudo pip3 install piexif==1.1.3
sudo pip3 install Pillow==8.1.2
sudo pip3 install pip==20.3.4
sudo pip3 install python-apt==2.2.1
sudo pip3 install python-prctl==1.7
sudo pip3 install requests==2.25.1
sudo pip3 install RPi.GPIO==0.7.0
sudo pip3 install setuptools==52.0.0
sudo pip3 install simplejpeg==1.6.4
sudo pip3 install six==1.16.0
sudo pip3 install spidev==3.5
sudo pip3 install ssh-import-id==5.10
sudo pip3 install toml==0.10.1
sudo pip3 install ufw==0.36
sudo pip3 install urllib3==1.26.5
sudo pip3 install v4l2-python3==0.3.2
sudo pip3 install websockets==12.0
sudo pip3 install wheel==0.34.2

# Clone the project repository
git clone https://github.com/groskowski/Raspi_XP.git ~/

# Replace configuration files
sudo cp ~/config/rc.local /etc/rc.local
sudo cp ~/config/dhcpcd.conf /etc/dhcpcd.conf
sudo cp ~/config/wpa_supplicant.conf /etc/wpa_supplicant/wpa_supplicant.conf
sudo cp ~/config/startup.sh /home/pi/startup.sh
sudo cp ~/config/sshd_config /etc/ssh/sshd_config

# Create commands
sudo cp check_status stop_php stop_python /usr/local/bin/
sudo chmod +x /usr/local/bin/check_status
sudo chmod +x /usr/local/bin/stop_php
sudo chmod +x /usr/local/bin/stop_python

# Make the startup script executable
sudo chmod +x /home/pi/config/startup.sh

# Set up ESP32 firmware
#mkdir -p ~/esp
#cd ~/esp
#git clone --recursive https://github.com/espressif/esp-idf.git

# Prompt for installing ESP-IDF
read -p "Do you want to install ESP-IDF now? (y/n): " install_choice
if [[ $install_choice =~ ^[Yy]$ ]]; then
	sudo apt-get install -y flex bison gperf python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0
    cd ~/esp/esp-idf
    ./install.sh esp32
    . $HOME/esp/esp-idf/export.sh
    
    # Prompt for building ESP-IDF examples
    read -p "Do you want to build code/softAP/* now? (y/n): " build_choice
    if [[ $build_choice =~ ^[Yy]$ ]]; then
        cd ~/code/softAP
        idf.py set-target esp32
        idf.py build
        
        # Prompt for flashing ESP32 firmware
        read -p "Do you want to flash the ESP32 firmware now? (y/n): " flash_choice
        if [[ $flash_choice =~ ^[Yy]$ ]]; then
            read -p "Please enter the port where your ESP32 is connected (e.g., /dev/ttyUSB0). You can exit the monitor after completion with Ctrl+]: " esp32_port
            idf.py -p $esp32_port flash monitor
        fi
    fi
fi

# Configure SSH server
sudo systemctl enable ssh
sudo systemctl restart ssh

# Configure MariaDB
sudo mysql_secure_installation

# Create the ipro497 database
sudo mysql -e "CREATE DATABASE ipro497;"

# Create root user with password "raspberry" in MariaDB
sudo mysql -e "CREATE USER 'root'@'localhost' IDENTIFIED BY 'raspberry';"
sudo mysql -e "GRANT ALL PRIVILEGES ON *.* TO 'root'@'localhost' WITH GRANT OPTION;"
sudo mysql -e "FLUSH PRIVILEGES;"

# Reboot the Raspberry Pi
read -p "Setup complete. Do you want to reboot now to apply some required changes? (y/n): " reboot_choice
if [[ $reboot_choice =~ ^[Yy]$ ]]; then
    echo "Adeus..."
    sleep 2
    sudo reboot
else
    echo "Please reboot manually to apply the changes."
fi