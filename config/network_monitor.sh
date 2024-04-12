#!/bin/bash

while true; do
    if ! ping -q -c 1 -W 1 8.8.8.8 >/dev/null; then
        echo "Network connection lost. Attempting to reconnect..."
        sudo ifconfig wlan0 down
        sleep 10
        sudo ifconfig wlan0 up
        sudo systemctl restart wpa_supplicant
        echo "Reconnection attempt completed."
    fi
    sleep 25
done