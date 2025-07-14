#!/bin/sh

./waf configure --board  MatekL431-Periph
sleep 5

./waf AP_Periph
