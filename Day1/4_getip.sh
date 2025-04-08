#!/bin/bash

ip=$(ip -4 addr show scope global| grep inet | awk '{print $2}' | cut -d/ -f1 | head -n 1)
if [ -n "$ip" ]; then
	echo "ip: $ip"
else
	echo "ip not found"
fi
