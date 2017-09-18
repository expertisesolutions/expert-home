// Copyright Felipe Magno de Almeida 2016.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_DEVICES_OUTPUT_MILIGHT_HPP
#define EXPERT_DEVICES_OUTPUT_MILIGHT_HPP

#include <boost/asio.hpp>
#include <functional>

namespace eh { namespace device { namespace output {

struct milight_device
{
  using endpoint_type = boost::asio::ip::udp::endpoint;
  
  milight_device(boost::asio::io_service& io_service
                 , endpoint_type endpoint)
    : io_service(&io_service), endpoint(endpoint)
    , socket(io_service)
  {
    socket.open(endpoint.protocol());
  }

  void watch(std::function<void(std::string, std::vector<argument_variant>)> callback) {}

  std::string type() const
  {
    return "MilightOutputLightRGB";
  }
  std::string var_type() const
  {
    return "string";
  }
  std::string gui_type() const
  {
    return "light_rgb";
    // return "scenario";
  }
  std::string state() const
  {
    return "0";
  }
  std::string io_type() const
  {
    return "output";
  }
  void send_command(std::string const& command, std::vector<argument_variant> const& args
                    , std::function<void(boost::system::error_code, std::string)> completion)
  {
    unsigned char message[3] = {0, 0, 0x55};
    if(command == "on")
      message[0] = 0x42;
    else if(command == "off")
      message[0] = 0x41;
    else
      return;

    std::cout << "sending command " << (int)message[0] << std::endl;
    
    socket.send_to(boost::asio::const_buffers_1(&message[0], sizeof(message)), endpoint);
    boost::system::error_code ec;
    completion(ec, "true");
  }

  boost::asio::io_service* io_service;
  endpoint_type endpoint;
  boost::asio::ip::udp::socket socket;

  friend std::ostream& operator<<(std::ostream& stream, milight_device const& d)
  {
    return stream;
  }
};
      
} } }

#endif
