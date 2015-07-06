-- Copyright Felipe Magno de Almeida 2015.
-- Distributed under the Boost Software License, Version 1.0.
--    (See accompanying file LICENSE_1_0.txt or copy at
--          http://www.boost.org/LICENSE_1_0.txt)

local device = {
   lg_ip = require('lua/devices/lg-ip'),
   lg_power_on = avail_devices.lg_power_on('lg_power_on', '192.168.20.196',
                                           '27546943',
                                           'felipe.m.almeida@gmail.com',
                                           'elF19le'),

   is_poweron = false,
}

function device.poweroff()
   device.lg_ip:poweroff()
   device.is_poweron = false
end
function device.poweron()
   device.lg_power_on:send_command('PowerOn', {})
   device.is_poweron = true
end
function device.channelup()
   device.lg_ip:channelup()
end
function device.channeldown()
   device.lg_ip:channeldown()
end
function device.channelback()
   device.lg_ip:channeldown()
end
function device.input_hdmi1()
   device.lg_ip:hdmi1()
end
function device.input_tv()
   device.lg_ip:input_tv()
end

return device
