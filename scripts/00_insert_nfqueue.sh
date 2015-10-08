#!/bin/bash

modprobe xt_NFQUEUE
modprobe nfnetlink
modprobe nfnetlink_queue

iptables -D INPUT  -j NFQUEUE --queue-num 0 2>/dev/null 
iptables -I INPUT  -j NFQUEUE --queue-num 0 
iptables -D OUTPUT -j NFQUEUE --queue-num 0 2>/dev/null
iptables -I OUTPUT -j NFQUEUE --queue-num 0 

iptables -L INPUT  -v -n | grep NFQ
iptables -L OUTPUT -v -n | grep NFQ

