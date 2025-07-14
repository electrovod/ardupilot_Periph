#!/bin/sh

echo ==== Build MUCAN -bootloader: 

# ./waf configure --board BVMT_MUCan --bootloader
# ./waf clean
# ./waf bootloader

Tools/scripts/build_bootloaders.py BVMT_MUCan

# Placed into :: Tools/bootloaders
