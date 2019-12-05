#!/bin/bash

ifconfig eth0 up
ifconfig eth0 172.16.20.1/24

#Add route
route add -net 172.16.21.0/24 gw 172.16.20.254