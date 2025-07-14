#!/bin/sh

echo ==== Build MUCAN: 

./waf configure --board  BVMT_MUCan
sleep 5

./waf AP_Periph
