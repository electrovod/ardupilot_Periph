#!/bin/sh

echo ==== Build AP_HW_BVMT_PDBex -bootloader: 

# ./waf configure --board BVMT_MUCan --bootloader
# ./waf clean
# ./waf bootloader

Tools/scripts/build_bootloaders.py BVMT_PDBex

# Placed into :: Tools/bootloaders
