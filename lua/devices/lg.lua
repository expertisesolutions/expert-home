-- Copyright Felipe Magno de Almeida 2015.
-- Distributed under the Boost Software License, Version 1.0.
--    (See accompanying file LICENSE_1_0.txt or copy at
--          http://www.boost.org/LICENSE_1_0.txt)

local device = {
   lg_ip = require('lua/devices/lg-ip'),
   is_poweron = false,
   on_poweron_c = {},
   on_poweroff_c = {},
   on_currentchannel_c = {},
   on_cursorvisible_c = {},
}

function device.callback ()
   print ('device lg harmony callback')
end

device.lg_power_on
   = avail_devices.lg_power_on('lg_power_on', '192.168.33.101',
                               '27546943',
                               'felipe.m.almeida@gmail.com',
                               'elF19le', device.callback)

device.lg_ip.on_poweron(function()
      if(not device.is_poweron) then
         device.is_poweron = true
         for i=1, #device.on_poweron_c do
            device.on_poweron_c[i] ()
         end
         device.on_poweron_c = {}
      end
end)
device.lg_ip.on_poweroff(function()
      if(device.is_poweron) then
         device.is_poweron = false
         for i=1, #device.on_poweroff_c do
            device.on_poweroff_c[i] ()
         end
         device.on_poweroff_c = {}
      end
end)
device.lg_ip.on_currentchannel(function(vars)
      for i=1, #device.on_currentchannel_c do
         device.on_currentchannel_c[i] (vars)
      end
      device.on_currentchannel_c = {}
end)
device.lg_ip.on_cursorvisible(function()
      for i=1, #device.on_cursorvisible_c do
         device.on_cursorvisible_c[i] ()
      end
      device.on_cursorvisible_c = {}
end)

function device.get_currentchannel()
   device.lg_ip.get_currentchannel()
end
function device.poweroff()
   device.lg_ip.poweroff()
end
function device.poweron()
   device.lg_power_on:send_command('PowerOn', {})
end
function device.channelup()
   device.lg_ip.channelup()
end
function device.channeldown()
   device.lg_ip.channeldown()
end
function device.channelback()
   device.lg_ip.channeldown()
end
function device.input_hdmi1()
   device.lg_ip.hdmi1()
end
function device.input_tv()
   device.lg_ip.input_tv()
end
function device.play()
   device.lg_ip.play()
end
function device.pause()
   device.lg_ip.pause()
end   
function device.fastforward()
   device.lg_ip.fastforward()
end   
function device.rewind()
   device.lg_ip.rewind()
end   
function device.stop()
   device.lg_ip.stop()
end   
function device.record()
   device.lg_ip.record()
end   
function device.directionup()
   device.lg_ip.directionup()
end   
function device.directiondown()
   device.lg_ip.directiondown()
end   
function device.directionleft()
   device.lg_ip.directionleft()
end   
function device.directionright()
   device.lg_ip.directionright()
end   
function device.home()
   device.lg_ip.home()
end   
function device.select()
   device.lg_ip.select()
end   
function device.back()
   device.lg_ip.back()
end   
function device.exit()
   device.lg_ip.exit()
end   
function device.touchmove(x, y)
   device.lg_ip.touchmove(x, y)
end   
function device.touchclick()
   device.lg_ip.touchclick()
end   

function device.on_poweron(f)
   table.insert(device.on_poweron_c, f)
end
function device.on_poweroff(f)
   table.insert(device.on_poweroff_c, f)
end
function device.on_currentchannel(f)
   table.insert(device.on_currentchannel_c, f)
end
function device.on_cursorvisible(f)
   table.insert(device.on_cursorvisible_c, f)
end

return device
