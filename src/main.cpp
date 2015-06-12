
#include <iostream>

#include <boost/asio.hpp>
#include <expert-home/devices/rs232.hpp>

int main()
{
  boost::asio::io_service io_service;

  eh::device::rs232 lg(io_service);

  lg.watch();
  
  io_service.run();
}
