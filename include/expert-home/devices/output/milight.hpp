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
    std::cout << "send command milight command is " << command << std::endl;
    unsigned char message[12] = {0, 0, 0x55, 0, };
    int size = 3;
    if(command == "on")
      {
        message[0] = 0x42;
        message[0] = 0x31;
        message[1] = 0;
        message[2] = 0;
        message[3] = 0x08;
        message[4] = 0x04;
        message[5] = 0x01;
        message[6] = 0x00;
        message[7] = 0x00;
        message[8] = 0x00;
        message[9] = 0x00;
        message[10] = 0x00;
        message[11] = 0x3E;
        size = 12;
      }
    else if(command == "off")
      message[0] = 0x41;
    else
      {
        message[0] = 0x31;
        message[1] = 0;
        message[2] = 0;
        message[3] = 7;
        message[4] = 1;
        message[5] = 0xFF;
        message[6] = 0;
        message[7] = 0;
        message[8] = 0;
        message[9] = 0;
        size = 10;
        //message[0] = 0x40;
        // [0x31,0x00,0x00,0x08,0x02,saturation,0x00,0x00,0x00,zoneID]
        // [0x31,0x00,0x00,0x08,0x01,color,color,color,color,zoneID]
      }

    std::cout << "sending command " << (int)message[0] << std::endl;
    
    socket.send_to(boost::asio::const_buffers_1(&message[0], size), endpoint);
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
