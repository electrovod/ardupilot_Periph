target extended-remote /dev/ttyACM0
monitor frequency 200k
set mem inaccessible-by-default off
monitor swdp_scan
attach 1
load ~/ArduPilot/build/BVMT_RIO/bin/AP_Periph
compare-sections
quit
