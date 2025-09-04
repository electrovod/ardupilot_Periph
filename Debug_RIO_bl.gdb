target extended-remote /dev/ttyACM0
monitor frequency 1200k
set mem inaccessible-by-default off
monitor swdp_scan
attach 1
load ~/ArduPilot/Tools/bootloaders/BVMT_RIO_bl.elf
compare-sections
kill
quit
