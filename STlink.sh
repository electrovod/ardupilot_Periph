#!/bin/sh
#
# Script to start STM32CubeProg programmer:
#

# xfce4-terminal --command="/usr/local/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32CubeProgrammer" 

env -u SESSION_MANAGER  xterm -e 

/usr/local/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32CubeProgrammer &





