// Copyright Felipe Magno de Almeida 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_HOME_DEVICES_LUA_AVR_HPP
#define EXPERT_HOME_DEVICES_LUA_AVR_HPP

#include <luabind/luabind.hpp>

namespace eh { namespace devices { namespace lua {

struct avr_impl_base
{
  virtual ~avr_impl_base();
  virtual void change_input(std::string const& input) = 0;
  virtual avr_impl_base* clone() const = 0;
};
      
template <typename T>
struct avr_impl : avr_impl_base
{
  avr_impl(T& impl)
    : impl(&impl) {}

  void change_input(std::string const& input)
  {
    impl->change_input(input);
  }

  avr_impl<T>* clone() const { return new avr_impl<T>(*this); }
  
  T* impl;
};
      
struct avr
{
  template <typename T>
  avr(T& impl)
    : impl(new avr_impl<T>(impl))
  {
  }
  ~avr()
  {
    delete impl;
  }
  avr(avr& other)
  {
    impl = other.impl->clone();
  }
  avr(avr const& other)
  {
    impl = other.impl->clone();
  }
  avr& operator=(avr const& other)
  {
    avr_impl_base* x = other.impl->clone();
    delete impl;
    impl = x;
    return *this;
  }
  avr(avr && other)
    : impl(other.impl)
  {
    other.impl = 0;
  }
  avr& operator==(avr&& other)
  {
    delete impl;
    impl = other.impl;
    other.impl = 0;
    return *this;
  }

  void change_input(std::string const& input)
  {
    impl->change_input(input);
  }
  
  avr_impl_base* impl;
};

void register_avr(lua_State* L)
{
  luabind::module(L)
  [
   luabind::class_<avr>("avr")
   .def("change_input", &avr::change_input)
  ]
  ;
}
      
} } }

#endif
