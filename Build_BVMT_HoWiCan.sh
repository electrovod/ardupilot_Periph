#!/bin/sh

echo ==== Build MUCAN: 

./waf configure --board  BVMT_HoWiCan
sleep 5

./waf AP_Periph
