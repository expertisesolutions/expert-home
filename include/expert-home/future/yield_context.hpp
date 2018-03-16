// Copyright Felipe Magno de Almeida 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_FUTURE_YIELD_CONTEXT_HPP
#define EXPERT_FUTURE_YIELD_CONTEXT_IP_HPP

#include <boost/asio.hpp>

namespace eh { namespace future {

template <typename T, typename ReadHandler>
BOOST_ASIO_INITFN_RESULT_TYPE(ReadHandler,
  void (boost::system::error_code, T))
  async_then(boost::unique_future<T>&& future, ReadHandler&& handler)
{
  struct a
  {~a()
    {
      std::cout << "async_then destroying scope/returning" << std::endl;
      if(std::current_exception())
        std::cout << "An exception was thrown" << std::endl;
    }
  } a;
  
  boost::asio::detail::async_result_init
    <ReadHandler, void(boost::system::error_code, T)> init
    (std::forward<ReadHandler>(handler));

  std::cout << "calling future.then" << std::endl;
  // something
  future.then([handler = std::move(init.handler)] (boost::unique_future<T> future) mutable
              {
                std::cout << "then was called" << std::endl;
                if(future.has_exception())
                  {
                    // what to do?
                    //std::rethrow_exception(future.get_exception_ptr());
                    // how to rethrow?
                  }
                else
                  {
                    //strand.post([&] { handler(boost::system::error_code{}, future.get()); });
                    handler(boost::system::error_code{}, future.get());
                  }
              });
  std::cout << "called future.then" << std::endl;

  T x = init.result.get();
  std::cout << "returning async_wait" << std::endl;
  return x;
}

} }

#endif
