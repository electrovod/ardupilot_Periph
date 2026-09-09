#!/bin/sh

Target_Brd=BVMT_PDBex
echo ==== Configure : $Target_Brd ====

# First, add new Board to "./Tools/AP_Bootloader/board_types.txt" file

sleep 2
./waf configure --board $Target_Brd --debug   

echo ====  AP_Periph: ====
sleep 2

./waf AP_Periph
exit

# ./waf clean                                                                                                                   
# ./waf bootloader

echo ==== Build $Target_Brd : 

sleep 2

./waf AP_Periph
