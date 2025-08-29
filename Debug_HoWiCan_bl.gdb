target extended-remote /dev/ttyACM0
monitor swdp_scan
attach 1
load ./Tools/bootloaders/BVMT_HoWiCan_bl.elf
