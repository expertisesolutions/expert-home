// Copyright Felipe Magno de Almeida 2015-2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_DEVICES_HARMONY_DEVICE_IP_HPP
#define EXPERT_DEVICES_HARMONY_DEVICE_IP_HPP

#include <boost/asio.hpp>

#include <harmony/communication.hpp>

#include <expert-home/argument_variant.hpp>

namespace eh { namespace device {

struct harmony_device
{
  harmony_device(boost::asio::io_service& service
                 , boost::asio::ip::tcp::endpoint endpoint
                 , std::string_view hub_id, std::string_view client_id);

  void watch(std::function<void(std::string, std::vector<argument_variant>)> callback);
  void send_command(std::string const& command, std::vector<argument_variant> const& args
                    , std::function<void(boost::system::error_code, std::string)> handler);

  harmony::connection connection;
  harmony::communication communication;
  std::function<void(std::string, std::vector<argument_variant>)> callback;
};

std::ostream& operator<<(std::ostream& os, harmony_device const& device);

    
} }

#endif
