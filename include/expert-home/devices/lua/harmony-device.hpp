// Copyright Felipe Magno de Almeida 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_HOME_DEVICES_LUA_HARMONY_DEVICE_HPP
#define EXPERT_HOME_DEVICES_LUA_HARMONY_DEVICE_HPP

#include <expert-home/devices/harmony-device.hpp>
#include <expert-home/devices/lua/device.hpp>
#include <luabind/luabind.hpp>

namespace eh { namespace devices { namespace lua {

struct harmony_device
{
  harmony_device(eh::device::harmony_device& impl)
    : impl(&impl)
  {
  }
  ~harmony_device()
  {
  }
  harmony_device(harmony_device const& other)
  {
    impl = other.impl;
  }
  harmony_device& operator=(harmony_device const& other)
  {
    impl = other.impl;
    return *this;
  }
  harmony_device(harmony_device && other)
    : impl(other.impl)
  {
    other.impl = 0;
  }

  void send_command(std::string const& input
                    , std::vector<argument_variant> const& args)
  {
    std::function<void(boost::system::error_code, std::string)> f;
    impl->send_command(input, args, f);
  }
  
  eh::device::harmony_device* impl;
};

inline void register_harmony_device(lua_State* L)
{
  luabind::module(L)
  [
   luabind::class_<harmony_device>("harmony_device")
   .def("send_command", &device::send_command)
  ]
  ;
}
      
} } }

#endif
