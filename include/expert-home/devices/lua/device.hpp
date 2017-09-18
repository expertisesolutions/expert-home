// Copyright Felipe Magno de Almeida 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_HOME_DEVICES_LUA_DEVICE_HPP
#define EXPERT_HOME_DEVICES_LUA_DEVICE_HPP

#include <expert-home/devices/device.hpp>
#include <luabind/luabind.hpp>

namespace eh { namespace devices { namespace lua {

struct device
{
  device(eh::device::device_any& impl)
    : impl(&impl)
  {
  }
  ~device()
  {
  }
  device(device const& other)
  {
    impl = other.impl;
  }
  device& operator=(device const& other)
  {
    impl = other.impl;
    return *this;
  }
  device(device && other)
    : impl(other.impl)
  {
    other.impl = 0;
  }

  // std::string name() const
  // {
  //   return impl->name();
  // }
  std::string type() const
  {
    return impl->type();
  }
  std::string var_type() const
  {
    return impl->var_type();
  }
  std::string gui_type() const
  {
    return impl->gui_type();
  }
  std::string state() const
  {
    return impl->state();
  }
  std::string io_type() const
  {
    return impl->io_type();
  }
  void send_command(std::string const& input
                    , std::vector<argument_variant> const& args)
  {
    std::function<void(boost::system::error_code, std::string)> f;
    impl->send_command(input, args, f);
  }
  
  eh::device::device_any* impl;
};

inline int lua_send_command(lua_State* L)
{
  return 0;
}
      
inline void register_device(lua_State* L)
{
  luabind::module(L)
  [
   luabind::class_<device>("device")
   .def("send_command", &device::send_command)
   .def("type", &device::type)
   .def("var_type", &device::var_type)
   .def("state", &device::state)
   .def("io_type", &device::io_type)
   .def("gui_type", &device::gui_type)
  ]
  ;
}
      
} } }

#endif
