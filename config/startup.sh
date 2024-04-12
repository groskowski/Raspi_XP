#!/bin/bash

# Check if verbose flag is set
VERBOSE=0
if [[ "$1" == "--verbose" ]]; then
  VERBOSE=1
fi

# Function to optionally print messages
log() {
  if [[ $VERBOSE -eq 1 ]]; then
    echo "$@"
  fi
}

# Enable ufw
sudo ufw enable >/dev/null 2>&1
log "UFW enabled."

# Allow necessary ports
sudo ufw allow 22 >/dev/null 2>&1
log "Port 22 open."
sudo ufw allow 80 >/dev/null 2>&1
log "Port 80 open."
sudo ufw allow 8000 >/dev/null 2>&1
log "Port 8000 open."
sudo ufw allow 7000 >/dev/null 2>&1
log "Port 7000 open."

# Start the PHP server
cd /home/pi/code/Website
sudo php -S 0.0.0.0:8000 >/dev/null 2>&1 &
php_pid=$!
log "PHP server process started."

# Start the Python script
cd /home/pi/code
python DummySocket.py >/dev/null 2>&1 &
python_pid=$!
log "Python script process started."

# Wait for a few seconds to allow the servers to start
sleep 5

# Check if the PHP server is running
if ps -p $php_pid > /dev/null; then
    log "PHP server started successfully."
else
    echo "PHP server failed to start."
fi

# Check if the Python script is running
if ps -p $python_pid > /dev/null; then
    log "Python websocket at port 7000 started successfully."
else
    echo "Python websocket failed to start."
fi
