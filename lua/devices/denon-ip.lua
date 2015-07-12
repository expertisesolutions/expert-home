-- Copyright Felipe Magno de Almeida 2015.
-- Distributed under the Boost Software License, Version 1.0.
--    (See accompanying file LICENSE_1_0.txt or copy at
--          http://www.boost.org/LICENSE_1_0.txt)

local device = {
   is_masteron = false,
   is_secondon = false,
}

function device.callback()
   print ('denon device callback')
end

device.raw_device = avail_devices.denon('avr', 'denon', device.callback)

function device.poweroff()
   device.raw_device:send_command('ZM', {'OFF'})
end
function device.poweron()
   device.raw_device:send_command('ZM', {'ON'})
end
function device.channelup()
end
function device.channeldown()
end
function device.channelback()
end
function device.hdmi1()
end

return device
