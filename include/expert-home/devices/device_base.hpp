// Copyright Felipe Magno de Almeida 2016.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_HOME_DEVICES_DEVICE_BASE_HPP
#define EXPERT_HOME_DEVICES_DEVICE_BASE_HPP

namespace eh { namespace devices {

// struct device_impl_base
// {
//   virtual ~device_impl_base();
//   virtual std::string type() const = 0;
//   virtual std::string var_type() const = 0;
//   virtual std::string gui_type() const = 0;
//   virtual std::string state() const = 0;
//   virtual std::string io_type() const = 0;
//   virtual void send_command(std::string const& input
//                             , std::vector<argument_variant> const&) = 0;
//   virtual device_impl_base* clone() const = 0;
// };
      
// template <typename T>
// struct device_impl : device_impl_base
// {
//   device_impl(T& impl)
//     : impl(&impl) {}

//   void send_command(std::string const& input, std::vector<argument_variant> const& args)
//   {
//     impl->send_command(input, args);
//   }

//   device_impl<T>* clone() const { return new device_impl<T>(*this); }

//   std::string type() const
//   {
//     return impl->type();
//   }
//   std::string var_type() const
//   {
//     return impl->var_type();
//   }
//   std::string gui_type() const
//   {
//     return impl->gui_type();
//   }
//   std::string state() const
//   {
//     return impl->state();
//   }
//   std::string io_type() const
//   {
//     return impl->io_type();
//   }
  
//   T* impl;
// };

} }
    
#endif
