#!/bin/bash

# Check if verbose flag is set
VERBOSE=1
if [[ "$1" == "--verbose" ]]; then
  VERBOSE=1
fi

# Function to optionally print messages
log() {
  if [[ $VERBOSE -eq 1 ]]; then
    echo "$(date '+%Y-%m-%d %H:%M:%S') - $@"
  fi
}

log "Starting the setup process..."

# Check if UFW is already enabled
ufw_status=$(sudo ufw status | grep -o "Status: active")
if [[ $ufw_status == "Status: active" ]]; then
    log "UFW is already enabled."
else
    log "UFW is not enabled, enabling now..."
    sudo ufw enable -y >/dev/null 2>&1
fi

# Allow necessary ports
log "Allowing ports..."
sudo ufw allow 22 >/dev/null 2>&1
log "Port 22 open."
sudo ufw allow 80 >/dev/null 2>&1
log "Port 80 open."
sudo ufw allow 7000 >/dev/null 2>&1
log "Port 7000 open."

# Start the PHP server
log "Starting the PHP server..."
cd /home/pi/code/Website
sudo php -S 0.0.0.0:80 >/dev/null 2>&1 &
sleep 0.1
php_pid=$!
log "PHP server process started."

sleep 2

log "Restarting daemon..."
sudo killall pigpiod >/dev/null 2>&1 &
sudo pigpiod >/dev/null 2>&1 &

sleep 3

function check_py_status() {
    if ps -p $python_pid > /dev/null
    then
        log "Python script is running."
    else
        log "Python script has stopped. Restarting..."
        python3 /home/pi/code/python/main.py &
        python_pid=$!
        log "Python script restarted."
    fi
}

# Run the while loop in the background
(
    while true
    do
        check_py_status
        sleep 300
    done
) &

log "Startup script completed."