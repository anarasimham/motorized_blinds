#!/bin/bash

while true
do
    echo "Trying to start program"
    python3 -u ~/automated_blinds/host_proc/linux_send_time.py
    echo "Program exited. Sleeping"
    sleep 10
done
