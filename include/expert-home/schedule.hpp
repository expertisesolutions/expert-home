// Copyright Felipe Magno de Almeida 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EH_SCHEDULE_HPP
#define EH_SCHEDULE_HPP

#include <boost/asio.hpp>

#include <luabind/luabind.hpp>
#include <luabind/tag_function.hpp>

namespace eh {

void schedule_expired(boost::system::error_code const& ec, luabind::object function)
{
  std::cout << "schedule expired" << std::endl;
  function();
}
  
void schedule(boost::asio::io_service& service, unsigned int seconds, unsigned int minutes, unsigned int hours
              , luabind::object function)
{
  std::cout << "schedule in " << seconds << std::endl;
  boost::asio::deadline_timer* timer = new boost::asio::deadline_timer(service);
  timer->expires_from_now
    (boost::posix_time::seconds(seconds) + boost::posix_time::minutes(minutes)
     + boost::posix_time::hours(hours));
  timer->async_wait(std::bind(schedule_expired, std::placeholders::_1, function));
}

void register_schedule(boost::asio::io_service& service, lua_State* L)
{
  luabind::module(L)
  [
   luabind::def("schedule", luabind::tag_function<void(unsigned int, unsigned int, unsigned int
                                                       , luabind::object)>
                (std::bind(&schedule, std::ref(service), std::placeholders::_1
                           , std::placeholders::_2, std::placeholders::_3
                           , std::placeholders::_4)))
  ];
}
  
}

#endif
