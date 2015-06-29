// Copyright Felipe Magno de Almeida 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>

#include <boost/asio.hpp>
#include <expert-home/devices/rs232.hpp>
#include <expert-home/devices/denon-ip.hpp>
#include <expert-home/devices/lg-ip.hpp>
#include <expert-home/devices/lua/avr.hpp>
#include <expert-home/devices/lua/tv.hpp>

#include <lua.hpp>

#include <luabind/luabind.hpp>
#include <luabind/tag_function.hpp>

namespace eh { namespace devices { namespace lua {
avr_impl_base::~avr_impl_base() {}
} } }
namespace eh { namespace devices { namespace lua {
tv_impl_base::~tv_impl_base() {}
} } }

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

void print(lua_State* L, luabind::object device, std::string command, std::vector<eh::argument_variant> args)
{
  std::cout << "command " << command << " with " << args.size() << " arguments ";
  for(auto const& arg : args)
  {
    boost::apply_visitor(print_visit(), arg);
  }
  std::endl(std::cout);

  lua_getglobal(L, "device_handler");
  device.push(L);
  lua_pushstring(L, command.c_str());
  for(auto const& arg : args)
  {
    boost::apply_visitor(lua_push_visitor{L}, arg);
  }
  if(lua_pcall(L, 2 + args.size(), 0, 0))
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

  luabind::open(L);
  eh::devices::lua::register_avr(L);
  eh::devices::lua::register_tv(L);

  std::map<std::string, eh::device::lg_ip> lgs;
  std::map<std::string, eh::device::denon_ip> denons;
  
  luabind::module(L, "devices")
  [
     luabind::def("lg",
                  luabind::tag_function<luabind::object(std::string, std::string, std::string)>
                  ([&] (std::string name, std::string hostname, std::string pass) -> luabind::object
                  {
                    auto iterator = lgs.emplace
                      (name, eh::device::lg_ip{io_service, hostname, pass}).first;
                    luabind::object lg_obj(L, eh::devices::lua::tv(iterator->second));
                    signal.connect(std::bind(&::print, L, lg_obj, std::placeholders::_1, std::placeholders::_2));
                    iterator->second.watch(signal);
                    return lg_obj;
                  }))
   , luabind::def("denon",
                  luabind::tag_function<luabind::object(std::string, std::string)>
                  ([&] (std::string name, std::string hostname) -> luabind::object
                  {
                    auto iterator = denons.emplace
                      (name, eh::device::denon_ip{io_service, hostname}).first;
                    luabind::object denon_obj(L, eh::devices::lua::avr(iterator->second));
                    signal.connect(std::bind(&::print, L, denon_obj, std::placeholders::_1, std::placeholders::_2));
                    iterator->second.watch(signal);
                    return denon_obj;
                  }))
  ];

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

  std::cout << "watching denon" << std::endl;
  
  io_service.run();
}
