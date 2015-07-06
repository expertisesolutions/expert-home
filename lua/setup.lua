-- Copyright Felipe Magno de Almeida 2015.
-- Distributed under the Boost Software License, Version 1.0.
--    (See accompanying file LICENSE_1_0.txt or copy at
--          http://www.boost.org/LICENSE_1_0.txt)

--dofile('lua/devices/lg-ip.lua')

--package.path += ';/home/felipe/dev/home-automation/expert-home/lua/?.lua'

print ('Hello World')

devices =
{
   avr = require('lua/devices/denon-ip'),
   lg = require('lua/devices/lg'),
   roku =
      {
         device =avail_devices.roku_ip('roku', '192.168.20.1', 8060),
         
      }
}



print ('Hello World')
function device_handler(device, command, arg1, arg2, arg3, arg4)
   print ('lua command '.. command)
--    if(command == "MV") then
--       vol = arg1
--       if(arg1 > 99) then vol = arg1/10 end
--       print ('master volume '..vol)

-- --      devices.lg_power_on:change_input('1')
--    end

   --if(command == 'Home') then
      --print ('TV activity')
      --if(not devices.lg.is_poweron) then
         --devices.lg:poweron()
      --else
         --devices.lg:poweroff()
         --devices.lg:input_tv()
         
      --end
   --else

   --[[
   if(command == 'Left') then
      devices.lg:channeldown()
   elseif(command == 'Right') then
      devices.lg:channelup()
   elseif(command == 'Up') then
      devices.avr:poweroff()
      --devices.lg.lg_ip.raw_device:send_command('15', {})
   elseif(command == 'Down') then
      devices.avr:poweron()
      --devices.lg.lg_ip.raw_device:send_command('15', {})
   end
   --]]
end

