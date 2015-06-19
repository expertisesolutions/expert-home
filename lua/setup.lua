
print("Hello World")

function device_handler(command, arg1, arg2, arg3, arg4)
   print ('lua command '.. command)
   if(command == "MV") then
      vol = arg1
      if(arg1 > 99) then vol = arg1/10 end
      print ('master volume '..vol)
   end
end

