target extended-remote /dev/ttyACM0
monitor swdp_scan
attach 1
load ~/ArduPilot/build/BVMT_HoWiCan/bin/AP_Periph
compare-sections
