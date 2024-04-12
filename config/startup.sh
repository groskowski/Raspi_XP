#!/bin/bash

# Enable ufw
sudo ufw enable

# Allow necessary ports
sudo ufw allow 22
sudo ufw allow 80
sudo ufw allow 8000
sudo ufw allow 7000

# Start the PHP server
cd /home/pi/code/Website
sudo php -S 0.0.0.0:8000 &
php_pid=$!

# Start the Python script
cd /home/pi/code
python DummySocket.py &
python_pid=$!

# Wait for a few seconds to allow the servers to start
sleep 5

# Check if the PHP server is running
if ps -p $php_pid > /dev/null; then
    echo "PHP server started successfully"
else
    echo "PHP server failed to start"
fi

# Check if the Python script is running
if ps -p $python_pid > /dev/null; then
    echo "Python websocket at port 7000 started successfully"
else
    echo "Python websocket failed to start"
fi