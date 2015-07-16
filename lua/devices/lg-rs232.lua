
local device = {
   data = {
      is_poweron = false,
      is_videomuted = false,
      command = nil,
   },
   on_poweron_c = {},
   on_poweroff_c = {},
   on_currentchannel_c = {},
   on_videomutedon_c = {},
   on_videomutedoff_c = {},
}

function device.callback (d, command, arg1, arg2, arg3, arg4, arg5)
   if (device.data.command == 'k') then
      print('command was k')
      print('command is ', command, arg1, arg2, arg3, arg4)
      if(command == 'a') then
         if(arg3 == 0 and device.data.is_poweron) then
            device.data.is_poweron = false
            print('Now powered offf')
            for i=1, #device.on_poweroff_c do
               device.on_poweroff_c[i] ()
            end
            device.on_poweroff_c = {}
         elseif(arg3 == 1 and not device.data.is_poweron) then
            device.data.is_poweron = true
            print('Now powered on')
            for i=1, #device.on_poweron_c do
               device.on_poweron_c[i] ()
            end
            device.on_poweron_c = {}
         end
      elseif (command == 'd') then
         if(arg3 == 0 and device.data.is_videomuted) then
            device.data.is_videomuted = false
            for i=1, #device.on_videomutedon_c do
               device.on_videomutedon_c[i] ()
            end
            device.on_videomutedon_c = {}
         elseif (arg3 == 1 and not device.data.is_videomuted) then
            device.data.is_videomuted = true
            for i=1, #device.on_videomutedoff_c do
               device.on_videomutedoff_c[i] ()
            end
            device.on_videomutedoff_c = {}
         end
      end
   end
end

device.raw_device = avail_devices.lg_rs232('lg-rs232', '/dev/ttyUSB0', device.callback)

local function send_command(command, args)
   device.raw_device:send_command(command, {1, table.unpack(args)})
   device.data.command = string.sub(command, 1, 1)
   print('command is ', device.data.command)
end

function device.poweroff ()
   send_command('ka', {0})
end
function device.poweron ()
   send_command('ka', {1})
end
function device.videomutedon ()
   send_command('kd', {0})
end
function device.videomutedoff ()
   send_command('kd', {1})
end
function device.channelup ()
   send_command('mc', {0})
end
function device.channeldown ()
   send_command('mc', {1})
end
function device.volumeup ()
   send_command('mc', {2})
end
function device.volumedown ()
   send_command('mc', {3})
end
function device.channelback ()
   --send_command('HandleKeyInput', {'26'})
end
function device.input_hdmi1 ()
   send_command('kb', {8})
end
function device.input_tv ()
   send_command('kb', {0})
end
--function device.play()
--   send_command('HandleKeyInput', {'176'})
--end
--function device.pause()
--   send_command('HandleKeyInput', {'186'})
--end   
-- function device.fastforward()
--    send_command('HandleKeyInput', {'142'})
-- end   
-- function device.rewind()
--    send_command('HandleKeyInput', {'143'})
-- end   
function device.stop()
   send_command('mc', {0xbd})
end   
-- function device.record()
--    send_command('HandleKeyInput', {'189'})
-- end   
-- function device.get_currentchannel()
--    send_command('CurrentChannel', {})
-- end
function device.directionup()
   send_command('mc', {0x40})
end   
function device.directiondown()
   send_command('mc', {0x41})
end   
function device.directionleft()
   send_command('mc', {7})
end   
function device.directionright()
   send_command('mc', {6})
end   
function device.home()
   send_command('mc', {0x43})
end   
function device.select()
   send_command('mc', {0x44})
end   
function device.back()
   send_command('mc', {0x28})
end   
function device.exit()
   send_command('mc', {0x5b})
end   
function device.digit(i)
   send_command('mc', {0x10 + i})
end   
function device.green()
   send_command('mc', {0x71})
end   
function device.red()
   send_command('mc', {0x72})
end   
function device.red()
   send_command('mc', {0x72})
end   
function device.blue()
   send_command('mc', {0x61})
end   
function device.yellow()
   send_command('mc', {0x63})
end   

--function device.touchmove(x, y)
--   send_command('HandleTouchMove', {x, y})
--end   
--function device.touchclick()
--   send_command('HandleTouchClick', {})
--end   

function device.on_poweron(f)
   device.on_poweron_c[#device.on_poweron_c+1] = f
end
function device.on_poweroff(f)
   device.on_poweroff_c[#device.on_poweroff_c] = f
end
function device.on_videomutedon(f)
   device.on_videomutedon_c[#device.on_videomutedon_c+1] = f
end
function device.on_videomutedoff(f)
   device.on_videomutedoff_c[#device.on_videomutedoff_c] = f
end
function device.on_currentchannel(f)
   device.on_currentchannel_c[#device.on_currentchannel_c+1] = f
end
function device.is_poweron()
   send_command('ka', {0xff})
end
function device.is_videomutedon()
   send_command('kd', {0xff})
end

return device
