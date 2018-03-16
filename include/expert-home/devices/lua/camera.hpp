// Copyright Felipe Magno de Almeida 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_HOME_DEVICES_LUA_CAMERA_HPP
#define EXPERT_HOME_DEVICES_LUA_CAMERA_HPP

#include <expert-home/devices/device.hpp>
#include <luabind/luabind.hpp>

namespace eh { namespace devices { namespace lua {

struct camera
{
  std::string hostname;
  unsigned short port;
  int channel;
};

inline void register_camera(lua_State* L)
{
  luabind::module(L)
  [
   luabind::class_<camera>("camera")
  ]
  ;
}
      
} } }

#endif
