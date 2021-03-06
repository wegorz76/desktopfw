#!/bin/bash

modprobe xt_NFQUEUE
modprobe nfnetlink
modprobe nfnetlink_queue

iptables -D INPUT  -p udp --sport 53       -j NFQUEUE --queue-num 0 
iptables -I INPUT  -p udp --sport 53       -j NFQUEUE --queue-num 0 

iptables -D INPUT  -p tcp --sport 80       -j NFQUEUE --queue-num 0 
iptables -I INPUT  -p tcp --sport 80       -j NFQUEUE --queue-num 0 

#iptables -D OUTPUT -p tcp --dport 22 --syn -j NFQUEUE --queue-num 0 
#iptables -I OUTPUT -p tcp --dport 22 --syn -j NFQUEUE --queue-num 0 
#iptables -D OUTPUT -p udp --dport 53       -j NFQUEUE --queue-num 0 
#iptables -I OUTPUT -p udp --dport 53       -j NFQUEUE --queue-num 0 

iptables -L INPUT  -v -n | grep NFQ
iptables -L OUTPUT -v -n | grep NFQ

