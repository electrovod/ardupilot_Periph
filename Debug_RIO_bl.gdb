target extended-remote /dev/ttyACM0
monitor swdp_scan
attach 1
load ~/ArduPilot/Tools/bootloaders/BVMT_RIO_bl.elf

