#!/bin/bash

ifconfig eth0 up
ifconfig eth0 172.16.21.1/24

#Add route
route add -net 172.16.20.0/24 gw 172.16.21.253

route add default gw 172.16.21.254

echo 1 > /proc/sys/net/ipv4/conf/eth0/accept_redirects
echo 1 > /proc/sys/net/ipv4/conf/all/eth0/accept_redirect