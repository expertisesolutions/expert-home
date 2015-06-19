// Copyright Felipe Magno de Almeida 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>

#include <boost/asio.hpp>
#include <expert-home/devices/rs232.hpp>
#include <expert-home/devices/denon-ip.hpp>

#include <lua.hpp>

struct print_visit
{
  void operator()(std::string const& string) const
  {
    std::cout << " [string:" << string << "]";
  }

  void operator()(int i) const
  {
    std::cout << " [int:" << i << "]";
  }
};

struct lua_push_visitor
{
  lua_State* L;
  
  void operator()(std::string const& string) const
  {
    lua_pushstring(L, string.c_str());
  }

  void operator()(int i) const
  {
    lua_pushnumber(L, i);
  }
};

void print(lua_State* L, std::string command, std::vector<eh::argument_variant> args)
{
  std::cout << "command " << command << " with " << args.size() << " arguments ";
  for(auto const& arg : args)
  {
    boost::apply_visitor(print_visit(), arg);
  }
  std::endl(std::cout);

  lua_getglobal(L, "device_handler");
  lua_pushstring(L, command.c_str());
  for(auto const& arg : args)
  {
    boost::apply_visitor(lua_push_visitor{L}, arg);
  }
  if(lua_pcall(L, 1 + args.size(), 0, 0))
    {
      std::cout << "failed calling device handler" << std::endl;
    }
}

int main()
{
  boost::asio::io_service io_service;

  boost::signals2::signal<void(std::string, std::vector<eh::argument_variant>)> signal;

  lua_State* L = luaL_newstate();
  luaL_openlibs(L);
  if(luaL_loadfile(L, "lua/setup.lua"))
    {
      std::cout << "Error loading lua setup.lua" << std::endl;
      return -1;
    }

  if(lua_pcall(L, 0, 0, 0))
    {
      std::cout << "Error running lua setup.lua" << std::endl;
      return -1;
    }

  signal.connect(std::bind(&::print, L, std::placeholders::_1, std::placeholders::_2));
  
  // eh::device::rs232 lg(io_service, "/dev/ttyACM0");
  // lg.watch(signal);
  eh::device::denon_ip avr(io_service, "denon");
  avr.watch(signal);

  std::cout << "watching denon" << std::endl;
  
  io_service.run();
}
