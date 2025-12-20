target extended-remote /dev/ttyACM0
monitor frequency 250k
set mem inaccessible-by-default off
monitor swdp_scan
attach 1
load ~/ArduPilot/Tools/bootloaders/BVMT_CanEx_bl.elf
compare-sections
kill
quit
