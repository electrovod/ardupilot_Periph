#!/bin/sh 
# 
# Script to start CandleLite -> SocketCAN interface
#

sudo ip link set can0 up type can bitrate 1000000
sleep 1
ip link show can0
sleep 1

