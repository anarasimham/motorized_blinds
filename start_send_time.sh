#!/bin/bash
i=0

while [ $i -lt 1000 ]
do
    echo "Trying to start program"
    python3 -u ~/automated_blinds/host_proc/linux_send_time.py 2>&1 | tee ~/send_time.out
    echo "Program exited. Sleeping. i=$i"
    sleep 5
    ((i=i+1))
done
