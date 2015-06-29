// Copyright Felipe Magno de Almeida 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_HOME_DEVICES_LUA_TV_HPP
#define EXPERT_HOME_DEVICES_LUA_TV_HPP

#include <luabind/luabind.hpp>

namespace eh { namespace devices { namespace lua {

struct tv_impl_base
{
  virtual ~tv_impl_base();
  virtual void change_input(std::string const& input) = 0;
  virtual tv_impl_base* clone() const = 0;
};
      
template <typename T>
struct tv_impl : tv_impl_base
{
  tv_impl(T& impl)
    : impl(&impl) {}

  void change_input(std::string const& input)
  {
    impl->change_input(input);
  }

  tv_impl<T>* clone() const { return new tv_impl<T>(*this); }
  
  T* impl;
};
      
struct tv
{
  template <typename T>
  tv(T& impl)
    : impl(new tv_impl<T>(impl))
  {
  }
  ~tv()
  {
    delete impl;
  }
  tv(tv& other)
  {
    impl = other.impl->clone();
  }
  tv(tv const& other)
  {
    impl = other.impl->clone();
  }
  tv& operator=(tv const& other)
  {
    tv_impl_base* x = other.impl->clone();
    delete impl;
    impl = x;
    return *this;
  }
  tv(tv && other)
    : impl(other.impl)
  {
    other.impl = 0;
  }
  tv& operator==(tv&& other)
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
  
  tv_impl_base* impl;
};

void register_tv(lua_State* L)
{
  luabind::module(L)
  [
   luabind::class_<tv>("tv")
   .def("change_input", &tv::change_input)
  ]
  ;
}
      
} } }

#endif
