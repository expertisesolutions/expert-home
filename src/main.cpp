// Copyright Felipe Magno de Almeida 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>

#include <boost/asio.hpp>
#include <expert-home/devices/rs232.hpp>
#include <expert-home/devices/denon-ip.hpp>

struct print_visit
{
  void operator()(std::string const& string) const
  {
    std::cout << " [string:" << string << "]";
  }

  void operator()(int i) const
  {
    std::cout << " [int:" << i << "]";
  }
};

void print(std::string command, std::vector<eh::argument_variant> args)
{
  std::cout << "command " << command << " with " << args.size() << " arguments ";
  for(auto const& arg : args)
  {
    boost::apply_visitor(print_visit(), arg);
  }
  std::endl(std::cout);
}

int main()
{
  boost::asio::io_service io_service;

  boost::signals2::signal<void(std::string, std::vector<eh::argument_variant>)> signal;
  signal.connect(&::print);
  
  // eh::device::rs232 lg(io_service, "/dev/ttyACM0");
  // lg.watch(signal);
  eh::device::denon_ip avr(io_service, "denon");
  avr.watch(signal);

  std::cout << "watching denon" << std::endl;
  
  io_service.run();
}
