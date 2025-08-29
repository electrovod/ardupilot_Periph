#!/bin/sh

Target_Brd=BVMT_RIO
echo ==== Configure : $Target_Brd ====

# First, add new Board to "./Tools/AP_Bootloader/board_types.txt" file

sleep 2
./waf configure --board $Target_Brd --debug   

# --vs-launch

# BootLoader:
# echo ==== BootLoader: ====
# Tools/scripts/build_bootloaders.py BVMT_RIO
# exit

# ./waf clean                                                                                                                   
# ./waf bootloader

echo ==== Build $Target_Brd : 

sleep 2

./waf AP_Periph
