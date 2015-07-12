
--[[ 
1 -> channel-down
2 -> vol-up
3 -> vol-down
4 -> 
5 -> 
6 ->
7 -> record list?
8 -> power off
9 -> mute
10 -> quick menu
11 -> input button
12 ->
13 ->
14 -> sleep function
15 -> status (?)
16 -> 0
17 -> 1
18 -> 2
19 -> 3
20 -> 4
21 -> 5
22 -> 6
23 -> 7
24 -> 8
25 -> 9
26 -> channel-up
27 ->
28 ->
29 ->
30 -> FAV
31 ->
32 ->
33 ->
34 ->
35 -> status (?)
36 ->
37 ->
38 ->
39 ->
40 ->
41 ->
42 ->
43 ->
44 ->
45 ->
46 ->
47 ->
48 -> av mode
49 ->
50 ->
51 ->
52 ->
53 ->
54 ->
55 ->
56 ->
57 -> subtitles
58 -> 
59 ->
60 ->
61 ->
62 ->
63 ->
64 ->
65 ->
66 ->
67 -> home
68 -> enter
69 -> quick menu ?
70 ->
71 ->
72 ->
73 ->
74 ->
75 -> 
--]]

--[[
$menus = LgRemote::OrderedHash[
  :status_bar, 35,
  :quick_menu, 69,
  :home_menu, 67,
  :premium_menu, 89,
  :installation_menu, 207,
  :factory_advanced_menu1, 251,
  :factory_advanced_menu2, 255,
]

$power_controls = LgRemote::OrderedHash[
  :power_off, 8,
  :sleep_timer, 14,
]

$navigation = LgRemote::OrderedHash[
  :left, 7,
  :right, 6,
  :up, 64,
  :down, 65,
  :select, 68,
  :back, 40,
  :exit, 91,
  :red, 114,
  :green, 113,
  :yellow, 99,
  :blue, 97,
]

$keypad = LgRemote::OrderedHash[
  :"0", 16,
  :"1", 17,
  :"2", 18,
  :"3", 19,
  :"4", 20,
  :"5", 21,
  :"6", 22,
  :"7", 23,
  :"8", 24,
  :"9", 25,
  :underscore, 76,
]

$playback_controls = LgRemote::OrderedHash[
  :play, 176,
  :pause, 186,
  :fast_forward, 142,
  :rewind, 143,
  :stop, 177,
  :record, 189,
]

$input_controls = LgRemote::OrderedHash[
  :tv_radio, 15,
  :simplink, 126,
  :input, 11,
  :component_rgb_hdmi, 152,
  :component, 191,
  :rgb, 213,
  :hdmi, 198,
  :hdmi1, 206,
  :hdmi2, 204,
  :hdmi3, 233,
  :hdmi4, 218,
  :av1, 90,
  :av2, 208,
  :av3, 209,
  :usb, 124,
  :slideshow_usb1, 238,
  :slideshow_usb2, 168,
]

$tv_controls = LgRemote::OrderedHash[
  :channel_up, 0,
  :channel_down, 1,
  :channel_back, 26,
  :favorites, 30,
  :teletext, 32,
  :t_opt, 33,
  :channel_list, 83,
  :greyed_out_add_button?, 85,
  :guide, 169,
  :info, 170,
  :live_tv, 158,
]

$picture_controls = LgRemote::OrderedHash[
  :av_mode, 48,
  :picture_mode, 77,
  :ratio, 121,
  :ratio_4_3, 118,
  :ratio_16_9, 119,
  :energy_saving, 149,
  :cinema_zoom, 175,
  :"3d", 220,
  :factory_picture_check, 252,
]

$audio_controls = LgRemote::OrderedHash[
  :volume_up, 2,
  :volume_down, 3,
  :mute, 9,
  :audio_language, 10,
  :sound_mode, 82,
  :factory_sound_check, 253,
  :subtitle_language, 57,
  :audio_description, 145,
--]]

local device =
   {
      on_poweron_c = nil,
   }

function device.on_poweron(f)
   if(type(f) == 'function') then
      print('lg-ip poweron callback set for ', f)
      device.on_poweron_c = f
      print('lg-ip poweron callback set for ', device.on_poweron_c)
   else
      error('on_poweron must receive a function')
   end
end

function device.check_poweron()
   print('CHECK lg-ip poweron callback set for ', device.on_poweron_c)
end

function device.callback (d, command)
   print('real callback')
   if(command == 'poweron') then
      print('power command, lets see if we have callback', device.on_poweron_c)
      device.check_poweron()
      if(device.on_poweron_c) then
         print ('calling callback poweron')
         device.on_poweron_c ()
      end
   end
end

device.raw_device = avail_devices.lg('lg', 'tv', 'SMBDJA', device.callback)

function device.poweroff ()
   device.raw_device:send_command('HandleKeyInput', {'8'})
end
function device.channelup ()
   device.raw_device:send_command('HandleKeyInput', {'0'})
end
function device.channeldown ()
   device.raw_device:send_command('HandleKeyInput', {'1'})
end
function device.channelback ()
   device.raw_device:send_command('HandleKeyInput', {'26'})
end
function device.input_hdmi1 ()
   device.raw_device:send_command('HandleKeyInput', {'206'})
end
function device.input_tv ()
   device.raw_device:send_command('HandleKeyInput', {'15'})
end
function device.play()
   device.raw_device:send_command('HandleKeyInput', {'176'})
end
function device.pause()
   device.raw_device:send_command('HandleKeyInput', {'186'})
end   
function device.fastforward()
   device.raw_device:send_command('HandleKeyInput', {'142'})
end   
function device.rewind()
   device.raw_device:send_command('HandleKeyInput', {'143'})
end   
function device.stop()
   device.raw_device:send_command('HandleKeyInput', {'177'})
end   
function device.record()
   device.raw_device:send_command('HandleKeyInput', {'189'})
end   

return device

