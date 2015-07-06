// Copyright Felipe Magno de Almeida 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>

#include <boost/asio.hpp>
#include <expert-home/devices/harmony-device.hpp>
#include <expert-home/devices/rs232.hpp>
#include <expert-home/devices/denon-ip.hpp>
#include <expert-home/devices/lg-ip.hpp>
#include <expert-home/devices/input/roku-ip.hpp>
#include <expert-home/devices/lua/device.hpp>
#include <expert-home/devices/lua/input.hpp>

#include <lua.hpp>

#include <luabind/luabind.hpp>
#include <luabind/tag_function.hpp>

namespace eh { namespace devices { namespace lua {
device_impl_base::~device_impl_base() {}
} } }
namespace eh { namespace devices { namespace lua {
input_impl_base::~input_impl_base() {}
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

      std::cout << "Error: " << lua_tostring(L, -1) << std::endl;
    }
}

int main()
{
  boost::asio::io_service io_service;

  lua_State* L = luaL_newstate();
  luaL_openlibs(L);

  luabind::open(L);
  eh::devices::lua::register_device(L);
  eh::devices::lua::register_input(L);

  std::map<std::string, eh::device::lg_ip> lgs;
  std::map<std::string, eh::device::denon_ip> denons;
  std::map<std::string, eh::device::harmony_device> harmony_devices;
  std::map<std::string, eh::device::roku_ip> roku_ips;
  
  luabind::module(L, "avail_devices")
  [
     luabind::def("lg",
                  luabind::tag_function<luabind::object(std::string, std::string, std::string)>
                  ([&] (std::string name, std::string hostname, std::string pass) -> luabind::object
                  {
                    auto iterator = lgs.emplace
                      (name, eh::device::lg_ip{io_service, hostname, pass}).first;
                    luabind::object lg_obj(L, eh::devices::lua::device(iterator->second));
                    iterator->second.watch(std::bind(&::print, L, lg_obj, std::placeholders::_1, std::placeholders::_2));
                    return lg_obj;
                  }))
   , luabind::def("denon",
                  luabind::tag_function<luabind::object(std::string, std::string)>
                  ([&] (std::string name, std::string hostname) -> luabind::object
                  {
                    auto iterator = denons.emplace
                      (name, eh::device::denon_ip{io_service, hostname}).first;
                    luabind::object denon_obj(L, eh::devices::lua::device(iterator->second));
                    iterator->second.watch(std::bind(&::print, L, denon_obj, std::placeholders::_1, std::placeholders::_2));
                    return denon_obj;
                  }))
   , luabind::def("lg_power_on",
                  luabind::tag_function<luabind::object(std::string, std::string, std::string, std::string, std::string)>
                  ([&] (std::string name, std::string hostname, std::string device
                        , std::string email, std::string password) -> luabind::object
                  {
                    auto iterator = harmony_devices.emplace
                      (name, eh::device::harmony_device{io_service, hostname, device, email, password}).first;
                    luabind::object harmony_obj(L, eh::devices::lua::device(iterator->second));
                    iterator->second.watch(std::bind(&::print, L, harmony_obj, std::placeholders::_1, std::placeholders::_2));
                    return harmony_obj;
                  }))
   , luabind::def("roku_ip",
                  luabind::tag_function<luabind::object(std::string, std::string, unsigned short)>
                  ([&] (std::string name, std::string listen_ip, unsigned short port) -> luabind::object
                  {
                    auto iterator = roku_ips.emplace
                      (name, eh::device::roku_ip{io_service, listen_ip, port}).first;
                    luabind::object roku_obj(L, eh::devices::lua::input(iterator->second));
                    iterator->second.watch(std::bind(&::print, L, roku_obj, std::placeholders::_1, std::placeholders::_2));
                    return roku_obj;
                  }))
  ];

  if(luaL_loadfile(L, "lua/setup.lua"))
  {
    std::cout << "Error loading lua setup.lua" << std::endl;
    std::cout << "Error: " << lua_tostring(L, -1) << std::endl;
    return -1;
  }

  if(lua_pcall(L, 0, 0, 0))
  {
    std::cout << "Error running lua setup.lua" << std::endl;

    std::cout << "Error: " << lua_tostring(L, -1) << std::endl;
    
    return -1;
  }

  std::cout << "watching denon" << std::endl;
  
  io_service.run();
}
