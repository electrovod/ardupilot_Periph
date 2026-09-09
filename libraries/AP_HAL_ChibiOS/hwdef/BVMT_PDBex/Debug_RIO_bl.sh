#!/bin/sh

Target_Brd=BVMT_RIO
echo ==== Debug : $Target_Brd ====
echo -
echo ....Then, commands are:
echo target extended-remote /dev/ttyACM0
echo monitor swdp_scan
echo attach 1
echo load ~/ArduPilot/build/$Target_Brd/bin/AP_Periph
echo -

arm-none-eabi-gdb -x Debug_Erase.gdb   ~/ArduPilot/Tools/bootloaders/BVMT_RIO_bl.elf

# -x Debug_RIO_bl.gdb
# 
# Also, use:
# 		/build/BVMT_RIO/openocd.cfg

sleep 1
