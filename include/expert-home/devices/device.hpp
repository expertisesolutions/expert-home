// Copyright Felipe Magno de Almeida 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_HOME_DEVICES_DEVICE_HPP
#define EXPERT_HOME_DEVICES_DEVICE_HPP

#include <boost/type_erasure/any.hpp>
#include <boost/type_erasure/operators.hpp>
#include <boost/mpl/vector.hpp>

namespace eh { namespace device {

namespace type_erasure = boost::type_erasure;
namespace mpl = boost::mpl;

namespace detail {

template <typename C>
struct has_type
{
  static std::string apply(C const& cont) { return cont.type(); }
};
template <typename C>
struct has_var_type
{
  static std::string apply(C const& cont) { return cont.var_type(); }
};
template <typename C>
struct has_gui_type
{
  static std::string apply(C const& cont) { return cont.gui_type(); }
};
template <typename C>
struct has_state
{
  static std::string apply(C const& cont) { return cont.state(); }
};
template <typename C>
struct has_io_type
{
  static std::string apply(C const& cont) { return cont.io_type(); }
};
template <typename C>
struct has_send_command
{
  static void apply(C& cont, std::string const& input
                    , std::vector<argument_variant> const& args
                    , std::function<void(boost::system::error_code, std::string)> completion)
  { return cont.send_command(input, args, completion); }
};
  
// template <typename C>
// struct has_device_functions
// {
// };

}
    
template <typename C>
struct requirements : mpl::vector
  <
   type_erasure::ostreamable<std::ostream&, C>
  , type_erasure::constructible<C(C&&)>
  , type_erasure::destructible<C>
  // , detail::has_device_functions<C>
  , detail::has_type<C>
  , detail::has_var_type<C>
  , detail::has_gui_type<C>
  , detail::has_state<C>
  , detail::has_io_type<C>
  , detail::has_send_command<C>
  >
{};

typedef boost::type_erasure::any<eh::device::requirements<boost::type_erasure::_self>> device_any;
    
} }

namespace boost { namespace type_erasure {

// template <typename C, typename Base>
// struct concept_interface< ::eh::device::detail::has_device_functions<C>, Base, C> : Base
// {
//   std::string type() const { return call( ::eh::device::detail::has_type<C>{}, *this); }
//   std::string var_type() const { return call( ::eh::device::detail::has_var_type<C>{}, *this); }
//   std::string gui_type() const { return call( ::eh::device::detail::has_gui_type<C>{}, *this); }
//   std::string state() const { return call( ::eh::device::detail::has_state<C>{}, *this); }
//   std::string io_type() const { return call( ::eh::device::detail::has_io_type<C>{}, *this); }
//   void send_command(std::string const& input, std::vector< ::eh::argument_variant>const& args)
//   { return call( ::eh::device::detail::has_send_command<C>{}, *this, input, args); }
// };

template <typename C, typename Base>
struct concept_interface< ::eh::device::detail::has_type<C>, Base, C> : Base
{
  std::string type() const { return call( ::eh::device::detail::has_type<C>{}, *this); }
};

template <typename C, typename Base>
struct concept_interface< ::eh::device::detail::has_var_type<C>, Base, C> : Base
{
  std::string var_type() const { return call( ::eh::device::detail::has_var_type<C>{}, *this); }
};

template <typename C, typename Base>
struct concept_interface< ::eh::device::detail::has_gui_type<C>, Base, C> : Base
{
  std::string gui_type() const { return call( ::eh::device::detail::has_gui_type<C>{}, *this); }
};

template <typename C, typename Base>
struct concept_interface< ::eh::device::detail::has_state<C>, Base, C> : Base
{
  std::string state() const { return call( ::eh::device::detail::has_state<C>{}, *this); }
};

template <typename C, typename Base>
struct concept_interface< ::eh::device::detail::has_io_type<C>, Base, C> : Base
{
  std::string io_type() const { return call( ::eh::device::detail::has_io_type<C>{}, *this); }
};

template <typename C, typename Base>
struct concept_interface< ::eh::device::detail::has_send_command<C>, Base, C> : Base
{
  void send_command(std::string const& input, std::vector< ::eh::argument_variant>const& args
                    , std::function<void(boost::system::error_code, std::string)> completion)
  { return call( ::eh::device::detail::has_send_command<C>{}, *this, input, args, completion); }
};

} }


#endif


