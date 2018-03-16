// Copyright Felipe Magno de Almeida 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_HOME_CALAOS_LUA_CAMERA_HPP
#define EXPERT_HOME_CALAOS_LUA_CAMERA_HPP

#include <luabind/luabind.hpp>

#include <expert-home/argument_variant.hpp>

namespace eh { namespace calaos {

struct lua_camera
{
  luabind::object object;

  lua_camera(luabind::object object) : object(object) {}

  bool has_name() const
  {
    return luabind::type(object["name"]) != LUA_TNIL;
  }
  std::string name() const
  {
    std::cout << "calling name " << std::endl;
    auto name = object["name"];
    if(luabind::type(name) == LUA_TFUNCTION)
      return luabind::call_function<std::string>(name, object);
    else
      return luabind::object_cast<std::string>(name);
  }
  bool has_id() const
  {
    return luabind::type(object["id"]) != LUA_TNIL;
  }
  std::string id() const
  {
    std::cout << "calling name " << std::endl;
    return luabind::call_function<std::string>(object["id"], object);
  }
  bool has_ptz() const
  {
    std::cout << "calling  " << __func__ << std::endl;
    auto function = object["has_ptz"];
    if(luabind::type(function) == LUA_TFUNCTION)
      return luabind::call_function<bool>(function, object);
    else if(luabind::type(function) != LUA_TNIL)
      return luabind::object_cast<bool>(function);
    else return false;
  }
  // void send_command(std::string const& command, std::vector<argument_variant> const& args)
  // {
  //   luabind::call_function<void>(object["send_command"], object, command, args);
  // }
};
    
} }

#endif

