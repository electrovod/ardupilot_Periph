#!/bin/sh 
# 
# Script to start CandleLite -> SocketCAN interface
#

CanBaud=1000000

sudo ip link set can0 up type can bitrate $CanBaud
sleep 1
echo ==== CanBaud = $CanBaud
ip link show can0
sleep 1

