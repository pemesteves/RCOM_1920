#!/bin/bash

ifconfig eth0 up
ifconfig eth0  172.16.21.1/24

#Add route
route add -net 172.16.20.0/24 g2 172.16.21.253