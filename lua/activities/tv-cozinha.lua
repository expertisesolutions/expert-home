
local activity =
   {
      devices = nil,
   }

function activity.set_devices(devices)
   activity.devices = devices
end

function activity.start ()
   if(not devices.lg.is_poweron) then
      devices.lg.lg_ip.on_poweron(function ()
            devices.lg.input_tv()

            make_poweron(activities.watch_tv_sala)
            activities.current = activities.watch_tv_sala
      end)
      devices.lg.poweron()
   else
      make_poweron(activities.watch_tv_sala)
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

function activity.stop ()
   devices.avr.master.poweroff()
   devices.lg.poweroff()
end

function activity.buttons(quote1, command)
   if(quote1 == nil) then
      --if(command == 'Info') then
         --activities.watch_tv_sala.is_infopressed = true
      --if(command == 'BackSpace') then
         --activities.watch_tv_sala.is_backspacepressed = true
      if(command == 'Up') then
         devices.lg.channelup()
      elseif(command == 'Down') then
         devices.lg.channeldown()
      elseif(command == 'Left') then
         devices.avr.master.volumedown()
      elseif(command == 'Right') then
         devices.avr.master.volumeup()
      elseif(command == 'InstantReplay') then
         activity.stop()
         return
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
   elseif(quote1 == 'Info') then
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
      --activities.watch_tv_sala.is_infopressed = false
   elseif(quote1 == 'BackSpace') then
      if(command == 'Play') then
         devices.lg.pause()
      elseif(command == 'Back') then
         devices.lg.exit()
      end
      --activities.watch_tv_sala.is_backspacepressed = false
   end
   if(not devices.avr.master.is_poweron) then
      -- was on power saving and shut off
      devices.avr.master.poweron()
   end
end

return activity

