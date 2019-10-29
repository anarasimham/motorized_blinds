#!/bin/bash
i=0

while [ $i -lt 1000 ]
do
    echo "Trying to start program"
    python3 ~/automated_blinds/host_proc/linux_send_time.py
    sleep 5
    ((i=i+1))
done
