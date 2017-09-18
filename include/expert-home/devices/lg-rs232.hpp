// Copyright Felipe Magno de Almeida 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_DEVICES_LG_RS232_HPP
#define EXPERT_DEVICES_LG_RS232_HPP

#include <boost/asio.hpp>

#include <functional>
#include <sstream>

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/include/vector.hpp>
#include <expert-home/argument_variant.hpp>

#include <iomanip>

namespace eh { namespace device {

struct lg_rs232 {

  lg_rs232(boost::asio::io_service& service
           , std::string device)
    : port(service), device(device)
  {
  }

  std::string type()
  {
    return "TV";
  }
  std::string var_type()
  {
    return "string";
  }
  std::string gui_type()
  {
    return "unknown";
  }
  std::string state()
  {
    return "0";
  }
  std::string io_type()
  {
    return "output";
  }
  void watch(std::function<void(std::string, std::vector<argument_variant>)> function)
  {
    callback = function;
    port.open(device);
    port.set_option(boost::asio::serial_port::baud_rate(115200));
    port.set_option(boost::asio::serial_port::parity(boost::asio::serial_port::parity::odd));
    port.set_option(boost::asio::serial_port::stop_bits(boost::asio::serial_port::stop_bits::one));
    port.set_option(boost::asio::serial_port::flow_control(boost::asio::serial_port::flow_control::none));
    port.set_option(boost::asio::serial_port::character_size(8));

    auto completion = [this] (boost::system::error_code ec, std::size_t size) -> std::size_t
      {
        return !!ec || (size && buffer[size-1] == 'x') ? 0 : boost::asio::detail::default_max_transfer_size;
      };
    read_callback
      = [this, completion] (boost::system::error_code ec, std::size_t size)
      {
        std::cout << "received at least one whole command" << std::endl;
        std::copy(buffer.begin(), buffer.begin() + size
                  , std::ostream_iterator<char>(std::cout));
        std::endl(std::cout);

        namespace x3 = boost::spirit::x3;
        namespace fusion = boost::fusion;
        fusion::vector5<std::string, int, char, char, int> general_attr;
        
        auto iterator = buffer.begin();
        if(boost::spirit::x3::parse
           (iterator, buffer.begin() + size
            , +(x3::char_ - ' ')
            >> x3::omit[' ']
            >> x3::hex
            >> x3::omit[' ']
            >> (x3::char_ >> x3::char_)
            >> x3::hex
            >> x3::omit["x"]
            , general_attr))
        {
          std::string ok;
          ok += fusion::at_c<2>(general_attr);
          ok += fusion::at_c<3>(general_attr);
          callback(fusion::at_c<0>(general_attr)
                   , std::vector<argument_variant>{{fusion::at_c<1>(general_attr)
                         , ok
                         , fusion::at_c<4>(general_attr)}});
        }
        else
        {
          std::cout << "Failed parsing " << std::endl;
        }
        
        boost::asio::async_read(port, boost::asio::mutable_buffers_1(buffer.begin(), buffer.size())
                                , completion
                                , read_callback);
      };
    boost::asio::async_read(port, boost::asio::mutable_buffers_1(buffer.begin(), buffer.size())
                            , completion
                            , read_callback);
  }

  void send_command(std::string command, std::vector<argument_variant> args)
  {
    std::cout << "send_command " << command << " arg1 " << args[0]
              << " arg2 " << args[1] << std::endl;
    std::stringstream message;
    message << command << ' '
            << std::hex
            << std::setfill('0') << std::setw(2) << boost::get<int>(args[0])
            << ' '
            << std::setfill('0') << std::setw(2) << boost::get<int>(args[1])
            << '\n';
    std::string buffer = message.str();
    std::cout << "sending" << std::endl;
    std::cout << buffer;
    boost::asio::write(port, boost::asio::const_buffers_1(&buffer[0], buffer.size()));
  }

  std::function<void(boost::system::error_code, std::size_t)> read_callback;
  std::array<char, 1024> buffer;
  boost::asio::serial_port port;
  std::function<void(std::string, std::vector<argument_variant>)> callback;
  std::string device;
};

} }

#endif
