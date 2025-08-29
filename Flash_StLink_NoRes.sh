#!/bin/sh
#
# Script to start STM32CubeProg programmer:
#

# xfce4-terminal --command="/usr/local/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32CubeProgrammer" 

/usr/local/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer.sh  -c port=SWD   -w $1 -run

# -vb 3  : Verbose

echo -e "\7 ==== Done ===="







