// Copyright Felipe Magno de Almeida 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_LG_RS232_HPP
#define EXPERT_LG_RS232_HPP

#include <boost/asio.hpp>

namespace eh { namespace device {

struct rs232 {

  rs232(boost::asio::io_service& service)
    : io_service(service)
  {
  }

  boost::asio::io_service& io_service;
};
      
} }

#endif

