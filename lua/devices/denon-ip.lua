-- Copyright Felipe Magno de Almeida 2015.
-- Distributed under the Boost Software License, Version 1.0.
--    (See accompanying file LICENSE_1_0.txt or copy at
--          http://www.boost.org/LICENSE_1_0.txt)

local device = {
   master = {
      is_poweron = false,
      on_poweron_c = {},
      on_poweroff_c = {},
      volume = 50,
   },
   zone2 = {
      is_poweron = false,
      on_poweron_c = {},
      on_poweroff_c = {},
      volume = 50,
   },
   is_poweron = false,
   on_poweron_c = {},
   on_poweroff_c = {},
}

function device.callback(d, command, arg1, arg2, arg3, arg4)
   print ('denon device callback')

   if (command == 'MV') then
      if(arg1 > 100) then
         device.master.volume = arg1 / 10
      else
         device.master.volume = arg1
      end         
      print ('new volume ', device.master.volume)
   elseif (command == 'ZM') then
      if(arg1 == 'ON' and not device.master.is_poweron) then
         device.master.is_poweron = true
         for i = 1, #device.master.on_poweron_c do
            device.master.on_poweron_c[i]()
         end
         device.master.on_poweron_c = {}
      elseif(arg1 == 'OFF' and device.master.is_poweron) then
         device.master.is_poweron = false
         for i = 1, #device.master.on_poweroff_c do
            device.master.on_poweroff_c[i]()
         end
         device.master.on_poweroff_c = {}
      end
   elseif (command == 'PW') then
      if(arg1 == 'ON' and not device.is_poweron) then
         device.is_poweron = true
         for i = 1, #device.on_poweron_c do
            device.on_poweron_c[i]()
         end
         device.on_poweron_c = {}
      elseif(arg1 == 'STANDBY' and device.is_poweron) then
         device.is_poweron = false
         for i = 1, #device.on_poweroff_c do
            device.on_poweroff_c[i]()
         end
         device.on_poweroff_c = {}
      end
   elseif (command == 'Z2') then
      -- zone2
      if(type(arg1) == 'number') then
         if(arg1 > 100) then
            device.zone2.volume = arg1 / 10
         else
            device.zone2.volume = arg1
         end
         print ('Z2 new volume ', device.zone2.volume)
      elseif(type(arg1) == 'string') then
         if(arg1 == 'ON' and not device.zone2.is_poweron) then
            device.zone2.is_poweron = true
            for i = 1, #device.zone2.on_poweron_c do
               device.zone2.on_poweron_c[i]()
            end
            device.zone2.on_poweron_c = {}
         elseif(arg1 == 'OFF' and device.zone2.is_poweron) then
            device.zone2.is_poweron = false
            for i = 1, #device.zone2.on_poweroff_c do
               device.zone2.on_poweroff_c[i]()
            end
            device.zone2.on_poweroff_c = {}
         end
      end
   end
end

device.raw_device = avail_devices.denon('avr', 'denon', device.callback)

function device.master.poweroff()
   device.raw_device:send_command('ZM', {'OFF'})
end
function device.master.poweron()
   device.raw_device:send_command('ZM', {'ON'})
end
function device.master.tvaudio()
   device.raw_device:send_command('SI', {'TV'})
end
function device.master.on_poweron(f)
   assert(type(f) == 'function')
   device.master.on_poweron_c[#device.master.on_poweron_c+1] = f
end
function device.master.volumeup()
   device.raw_device:send_command('MV', {'UP'})
end
function device.master.volumedown()
   device.raw_device:send_command('MV', {'DOWN'})
end
function device.master.mute_on()
   device.raw_device:send_command('MU', {'ON'})
end
function device.master.mute_off()
   device.raw_device:send_command('MU', {'OFF'})
end

-- zone2
function device.zone2.poweroff()
   device.raw_device:send_command('Z2', {'OFF'})
end
function device.zone2.poweron()
   print ('calling poweron on denon zone2')
   device.raw_device:send_command('Z2', {'ON'})
end
function device.zone2.tvaudio()
   print ('calling tvaudio on denon zone2')
   device.raw_device:send_command('Z2', {'TV'})
end
function device.zone2.on_poweron(f)
   assert(type(f) == 'function')
   device.zone2.on_poweron_c[#device.zone2.on_poweron_c+1] = f
end
function device.zone2.volumeup()
   device.raw_device:send_command('Z2', {'UP'})
end
function device.zone2.volumedown()
   device.raw_device:send_command('Z2', {'DOWN'})
end

-- general

function device.poweroff()
   device.raw_device:send_command('PW', {'STANDBY'})
end
function device.poweron()
   device.raw_device:send_command('PW', {'ON'})
end
function device.allzonestereo_on()
   device.raw_device:send_command('MN', {'ZST ON'})
end
function device.allzonestereo_off()
   device.raw_device:send_command('MN', {'ZST OFF'})
end
function device.on_poweron(f)
   assert(type(f) == 'function')
   device.on_poweron_c[#device.on_poweron_c+1] = f
end

return device
