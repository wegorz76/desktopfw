#!/bin/bash

modprobe xt_NFQUEUE
modprobe nfnetlink
modprobe nfnetlink_queue

iptables -D INPUT  -i eth0 -p tcp --dport 80 -j NFQUEUE --queue-num 0 2>/dev/null 
iptables -I INPUT  -i eth0 -p tcp --dport 80 -j NFQUEUE --queue-num 0 
iptables -D OUTPUT -o eth0 -p tcp --dport 80 -j NFQUEUE --queue-num 0 2>/dev/null
iptables -I OUTPUT -o eth0 -p tcp --dport 80 -j NFQUEUE --queue-num 0 

iptables -D INPUT  -i wlan0 -p tcp --dport 80 -j NFQUEUE --queue-num 0 2>/dev/null 
iptables -I INPUT  -i wlan0 -p tcp --dport 80 -j NFQUEUE --queue-num 0 
iptables -D OUTPUT -o wlan0 -p tcp --dport 80 -j NFQUEUE --queue-num 0 2>/dev/null
iptables -I OUTPUT -o wlan0 -p tcp --dport 80 -j NFQUEUE --queue-num 0 


iptables -L INPUT  -v -n | grep NFQ
iptables -L OUTPUT -v -n | grep NFQ

