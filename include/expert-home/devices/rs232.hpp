// Copyright Felipe Magno de Almeida 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_LG_RS232_HPP
#define EXPERT_LG_RS232_HPP

#include <boost/asio.hpp>

#include <boost/signals2/signal.hpp>

// namespace eh { namespace device {

// struct rs232 {

//   rs232(boost::asio::io_service& service
//         , const char* device)
//     : port(service, device), io_service(service)
//   {
//     boost::asio::async_read(port, boost::asio::mutable_buffers_1(buffer.begin(), buffer.size())
//                             , std::bind([] {return false;})
//                             , std::bind([] {}));
//   }

//   void watch(boost::signals2::signal<void(std::string, std::vector<>)>& signal)
//   {
    
//   }

//   void command(const char*)
//   {
    
//   }

//   std::array<char, 1024> buffer;
//   boost::asio::serial_port port;
//   boost::asio::io_service& io_service;
// };
      
// } }

#endif

