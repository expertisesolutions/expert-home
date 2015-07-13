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
      },
--   roku2 =
--      {
--      },
}

activities =
{
      watch_tv_sala =
         {
            is_poweron = false,
            is_infopressed = false,
         },
      watch_tv_cozinha =
         {
            is_poweron = false,
            is_infopressed = false,
         },
      current = nil,
}

function activities.watch_tv_sala.start()
   if(not devices.lg.is_poweron) then
      devices.lg.lg_ip.on_poweron(function ()
            activities.watch_tv_sala.is_poweron = true
            activities.current = activities.watch_tv_sala
            devices.lg.input_tv()
      end)
      devices.lg.poweron()
   else
      activities.watch_tv_sala.is_poweron = true
      activities.current = activities.watch_tv_sala
   end
   
   if(not devices.avr.master.is_poweron) then
      devices.avr.master.on_poweron(function()
            devices.avr.master.tvaudio()
      end)
      devices.avr.master.poweron()
   else
      devices.avr.master.tvaudio()
   end
end
function activities.watch_tv_sala.stop()
   devices.avr.master.poweroff()
   devices.lg.poweroff()
end
function activities.watch_tv_sala.buttons(command)
   print ('activities.watch_tv_sala.buttons')
   if(not activities.watch_tv_sala.is_infopressed
      and not activities.watch_tv_sala.is_backspacepressed) then
      if(command == 'Info') then
         activities.watch_tv_sala.is_infopressed = true
      elseif(command == 'BackSpace') then
         activities.watch_tv_sala.is_backspacepressed = true
      elseif(command == 'Up') then
         devices.lg.channelup()
      elseif(command == 'Down') then
         devices.lg.channeldown()
      elseif(command == 'Left') then
         devices.avr.master.volumedown()
      elseif(command == 'Right') then
         devices.avr.master.volumeup()
      elseif(command == 'InstantReplay') then
         activities.watch_tv_sala.stop()
      elseif(command == 'Home') then
         devices.lg.home()
      elseif(command == 'Back') then
         devices.lg.stop()
      elseif(command == 'Rev') then
         devices.lg.rewind()
      elseif(command == 'Play') then
         devices.lg.play()
      elseif(command == 'Fwd') then
         devices.lg.fastforward()
      elseif(command == 'Select') then
         devices.lg.select()
      end
   elseif(activities.watch_tv_sala.is_infopressed) then
      if(command == 'Up') then
         devices.lg.directionup()
      elseif(command == 'Down') then
         devices.lg.directiondown()
      elseif(command == 'Left') then
         devices.lg.directionleft()
      elseif(command == 'Right') then
         devices.lg.directionright()
      elseif(command == 'Play') then
         devices.lg.record()
      elseif(command == 'Back') then
         devices.lg.back()
      end
      activities.watch_tv_sala.is_infopressed = false
   elseif(activities.watch_tv_sala.is_backspacepressed) then
      if(command == 'Play') then
         devices.lg.pause()
      elseif(command == 'Back') then
         devices.lg.exit()
      end
      activities.watch_tv_sala.is_backspacepressed = false
   end
end

--- cozinha

function activities.watch_tv_cozinha.start()
   if(not devices.lg.is_poweron) then
      devices.lg.lg_ip.on_poweron(function ()
            activities.watch_tv_cozinha.is_poweron = true
            activities.current = activities.watch_tv_cozinha
            devices.lg.input_tv()
      end)
      devices.lg.poweron()
   else
      activities.watch_tv_cozinha.is_poweron = true
      activities.current = activities.watch_tv_cozinha
   end

   -- we have to start master and later use allzonestereo
   if(not devices.avr.master.is_poweron) then
      local f = function()
         schedule(2, 0, 0, function()
                     devices.avr.allzonestereo_on()
                     devices.avr.master.mute_on()
         end)
      end
      
      devices.avr.master.on_poweron(function()
            if(devices.avr.is_poweron) then
               f()
            end
      end)
      devices.avr.on_poweron(function()
            if(devices.avr.master.is_poweron) then
               f()
            end
      end)
      devices.avr.master.tvaudio()
   else
      --devices.avr.zone2.tvaudio()
      devices.avr.allzonestereo_on()
   end
end
function activities.watch_tv_cozinha.stop()
   devices.avr.zone2.poweroff()
   devices.lg.poweroff()
end
function activities.watch_tv_cozinha.buttons(command)
   print ('activities.watch_tv_cozinha.buttons')
   if(not activities.watch_tv_cozinha.is_infopressed
      and not activities.watch_tv_cozinha.is_backspacepressed) then
      if(command == 'Info') then
         activities.watch_tv_cozinha.is_infopressed = true
      elseif(command == 'BackSpace') then
         activities.watch_tv_cozinha.is_backspacepressed = true
      elseif(command == 'Up') then
         devices.lg.channelup()
      elseif(command == 'Down') then
         devices.lg.channeldown()
      elseif(command == 'Left') then
         devices.avr.zone2.volumedown()
      elseif(command == 'Right') then
         devices.avr.zone2.volumeup()
      elseif(command == 'InstantReplay') then
         activities.watch_tv_cozinha.stop()
      elseif(command == 'Home') then
         devices.lg.home()
      elseif(command == 'Back') then
         devices.lg.stop()
      elseif(command == 'Rev') then
         devices.lg.rewind()
      elseif(command == 'Play') then
         devices.lg.play()
      elseif(command == 'Fwd') then
         devices.lg.fastforward()
      elseif(command == 'Select') then
         devices.lg.select()
      end
   elseif(activities.watch_tv_cozinha.is_infopressed) then
      if(command == 'Up') then
         devices.lg.directionup()
      elseif(command == 'Down') then
         devices.lg.directiondown()
      elseif(command == 'Left') then
         devices.lg.directionleft()
      elseif(command == 'Right') then
         devices.lg.directionright()
      elseif(command == 'Play') then
         devices.lg.record()
      elseif(command == 'Back') then
         devices.lg.back()
      end
      activities.watch_tv_cozinha.is_infopressed = false
   elseif(activities.watch_tv_cozinha.is_backspacepressed) then
      if(command == 'Play') then
         devices.lg.pause()
      elseif(command == 'Back') then
         devices.lg.exit()
      end
      activities.watch_tv_cozinha.is_backspacepressed = false
   end
end


function device_handler(device, command, arg1, arg2, arg3, arg4)
   print ('lua command '.. command)
   if(command == 'launch' and arg1 == 1) then
      activities.watch_tv_sala.start()
      print ('command is launch, launching activity watch_tv_sala')
   elseif(command == 'launch' and arg1 == 2) then
      activities.watch_tv_cozinha.start()
      print ('command is launch, launching activity watch_tv_cozinha')
   elseif(activities.current) then
      activities.current.buttons(command)
   end
end
devices.roku.device = avail_devices.roku_ip('roku', '192.168.33.51', 8060, device_handler, 'P0A070000007')

devices.avr.master.on_poweron(function()
      --devices.avr.
end)

--devices.roku2.device = avail_devices.roku_ip('roku2', '192.168.33.202', 8060, roku2_device_handler, 'XAA060000333')

-- schedule(5, 0, 0,
--          function()
--             print ('powering on to record')

--             local currentchannel = function ()
--                   devices.lg.on_currentchannel(function(vars)
--                         if(vars.major ~= 65520) then
--                            print ('already in input tv')
                           
--                            if(vars.ch_type == 'terrestrial' and
--                               vars.major == 4 and vars.minor == 1) then
--                               -- is in GLOBO
--                               print ('already in GLOBO TV')
--                               devices.lg.record()
--                            else
--                               print (' not on globo ', vars.ch_type, ' '
--                                      , vars.major, ' ', vars.minor)
--                            end
--                         else
--                            print ('must change input to input tv')
--                            devices.lg.input_tv()
--                         end
--                   end)
--                   devices.lg.get_currentchannel()
--             end

--             if(not devices.lg.is_poweron) then
--                devices.lg.lg_ip.on_poweron(currentchannel)
--                devices.lg.poweron()
--             else
--                currentchannel()
--             end
-- end)

-- schedule(5, 0, 0, function()
--             activities.watch_tv_cozinha.start()
-- end)

-- schedule(60, 0, 0, function()
--             devices.avr.poweroff()
-- end)

