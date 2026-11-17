#!/usr/bin/python

import dronecan
from dronecan import uavcan

req = uavcan.equipment.actuator.ArrayCommand()
cmd = uavcan.equipment.actuator.Command()
cmd.actuator_id = 13
cmd.command_type = cmd.COMMAND_TYPE_UNITLESS
req.commands = [ cmd ]

# ..... Repeated part:   ................
# Value: -1.0 ... 0 ... +1.0  ::
cmd.command_value = 0.5
node.broadcast(req)


'''
Result:
cmd.actuator_id = 16

cmd.command_value = 1300

req.commands = [ cmd ]

node.broadcast(req)
'''

