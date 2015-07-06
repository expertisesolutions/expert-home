// Copyright Felipe Magno de Almeida 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_HOME_DEVICES_LUA_DEVICE_HPP
#define EXPERT_HOME_DEVICES_LUA_DEVICE_HPP

#include <luabind/luabind.hpp>

namespace eh { namespace devices { namespace lua {

struct device_impl_base
{
  virtual ~device_impl_base();
  virtual void send_command(std::string const& input
                            , std::vector<argument_variant> const&) = 0;
  virtual device_impl_base* clone() const = 0;
};
      
template <typename T>
struct device_impl : device_impl_base
{
  device_impl(T& impl)
    : impl(&impl) {}

  void send_command(std::string const& input, std::vector<argument_variant> const& args)
  {
    impl->send_command(input, args);
  }

  device_impl<T>* clone() const { return new device_impl<T>(*this); }
  
  T* impl;
};
      
struct device
{
  template <typename T>
  device(T& impl)
    : impl(new device_impl<T>(impl))
  {
  }
  ~device()
  {
    delete impl;
  }
  device(device& other)
  {
    impl = other.impl->clone();
  }
  device(device const& other)
  {
    impl = other.impl->clone();
  }
  device& operator=(device const& other)
  {
    device_impl_base* x = other.impl->clone();
    delete impl;
    impl = x;
    return *this;
  }
  device(device && other)
    : impl(other.impl)
  {
    other.impl = 0;
  }
  device& operator==(device&& other)
  {
    delete impl;
    impl = other.impl;
    other.impl = 0;
    return *this;
  }

  void send_command(std::string const& input
                    , std::vector<argument_variant> const& args)
  {
    impl->send_command(input, args);
  }
  
  device_impl_base* impl;
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
  ]
  ;
}
      
} } }

#endif
