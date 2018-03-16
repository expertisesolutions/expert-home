// Copyright Felipe Magno de Almeida 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_HOME_CALAOS_LUA_DEVICE_HPP
#define EXPERT_HOME_CALAOS_LUA_DEVICE_HPP

#include <luabind/luabind.hpp>

#include <expert-home/argument_variant.hpp>

namespace eh { namespace calaos {

struct lua_device
{
  luabind::object object;

  lua_device(luabind::object object) : object(object) {}

  bool has_name() const
  {
    return luabind::type(object["name"]) != LUA_TNIL;
  }
  std::string name() const
  {
    std::cout << "calling name " << std::endl;
    return luabind::call_function<std::string>(object["name"], object);
  }
  std::string type() const
  {
    std::cout << "calling  " << __func__ << std::endl;
    return luabind::call_function<std::string>(object["type"], object);
  }
  std::string var_type() const
  {
    std::cout << "calling  " << __func__ << std::endl;
    return luabind::call_function<std::string>(object["var_type"], object);
  }
  std::string gui_type() const
  {
    std::cout << "calling  " << __func__ << std::endl;
    return luabind::call_function<std::string>(object["gui_type"], object);
  }
  std::string state() const
  {
    std::cout << "calling  " << __func__ << std::endl;
    return luabind::call_function<std::string>(object["state"], object);
  }
  std::string io_type() const
  {
    std::cout << "calling  " << __func__ << std::endl;
    return luabind::call_function<std::string>(object["io_type"], object);
  }
  void send_command(std::string const& command, std::vector<argument_variant> const& args)
  {
    luabind::call_function<void>(object["send_command"], object, command, args);
  }
};
    
} }

#endif

