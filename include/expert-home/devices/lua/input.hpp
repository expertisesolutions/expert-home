// Copyright Felipe Magno de Almeida 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_HOME_DEVICES_LUA_INPUT_HPP
#define EXPERT_HOME_DEVICES_LUA_INPUT_HPP

#include <luabind/luabind.hpp>

namespace eh { namespace devices { namespace lua {

struct input_impl_base
{
  virtual ~input_impl_base();
  //virtual void change_input(std::string const& input) = 0;
  virtual input_impl_base* clone() const = 0;
};
      
template <typename T>
struct input_impl : input_impl_base
{
  input_impl(T& impl)
    : impl(&impl) {}

  // void change_input(std::string const& input)
  // {
  //   impl->change_input(input);
  // }

  input_impl<T>* clone() const { return new input_impl<T>(*this); }
  
  T* impl;
};
      
struct input
{
  template <typename T>
  input(T& impl)
    : impl(new input_impl<T>(impl))
  {
  }
  ~input()
  {
    delete impl;
  }
  input(input& other)
  {
    impl = other.impl->clone();
  }
  input(input const& other)
  {
    impl = other.impl->clone();
  }
  input& operator=(input const& other)
  {
    input_impl_base* x = other.impl->clone();
    delete impl;
    impl = x;
    return *this;
  }
  input(input && other)
    : impl(other.impl)
  {
    other.impl = 0;
  }
  input& operator==(input&& other)
  {
    delete impl;
    impl = other.impl;
    other.impl = 0;
    return *this;
  }

  // void change_input(std::string const& input)
  // {
  //   impl->change_input(input);
  // }
  
  input_impl_base* impl;
};

void register_input(lua_State* L)
{
  luabind::module(L)
  [
   luabind::class_<input>("input")
   // .def("change_input", &input::change_input)
  ]
  ;
}
      
} } }

#endif
