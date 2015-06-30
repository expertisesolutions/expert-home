
print ('Hello World')

devices = { --lg = avail_devices.lg('lg', 'tv', 'SMBDJA'),
   avr = avail_devices.denon('avr', 'denon'),
   lg_power_on = avail_devices.lg_power_on('lg_power_on', 'HarmonyHub',
                                           '27546943',
                                           'felipe.m.almeida@gmail.com',
                                           'elF19le'),
          }

function device_handler(device, command, arg1, arg2, arg3, arg4)
   print ('lua command '.. command)
   if(command == "MV") then
      vol = arg1
      if(arg1 > 99) then vol = arg1/10 end
      print ('master volume '..vol)

   end
end

devices.lg_power_on:change_input('1')
