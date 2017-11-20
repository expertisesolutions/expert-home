// Copyright Felipe Magno de Almeida 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <expert-home/devices/harmony-device.hpp>

namespace eh { namespace device {

harmony_device::harmony_device(boost::asio::io_service& service
                               , boost::asio::ip::tcp::endpoint endpoint
                               , std::string_view hub_id, std::string_view client_id)
  : connection(service, endpoint, hub_id, client_id)
  , communication(connection, hub_id, client_id)
{
}

void harmony_device::watch(std::function<void(std::string, std::vector<argument_variant>)> callback)
{
  communication.connect
    ([] (auto&) {});
  this->callback = callback;
}

void harmony_device::send_command(std::string const& command, std::vector<argument_variant> const& args
                                  , std::function<void(boost::system::error_code, std::string)> handler)
{
  std::cout << "sending command harmony " << command << std::endl;
}

std::ostream& operator<<(std::ostream& os, harmony_device const& device)
{
  return os << "[harmony device]";
}    
    
} }
