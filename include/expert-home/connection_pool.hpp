// Copyright Felipe Magno de Almeida 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_CONNECTION_MULTIPLEX_HPP
#define EXPERT_CONNECTION_MULTIPLEX_HPP

namespace eh { namespace device { namespace camera {

struct connection_pool
{
  connection_pool(boost::asio::ip::tcp::endpoint endpoint
                  , boost::asio::io_service& service)
    : endpoint(endpoint), socket(service), connected(false)
  {
    std::cout << "connection pool " << endpoint << std::endl;
  }

  boost::unique_future<boost::asio::ip::tcp::socket>
  get_socket()
  {
    if(connected)
    {
      std::cout << "get_socket queueing" << std::endl;
      boost::promise<boost::asio::ip::tcp::socket> promise;
      boost::unique_future<boost::asio::ip::tcp::socket> future = promise.get_future();
      queued_promises.push_back(std::move(promise));
      return future;
    }
    else
    {
      connected = true;

      boost::promise<boost::asio::ip::tcp::socket> promise;
      boost::unique_future<boost::asio::ip::tcp::socket> future = promise.get_future();
      queued_promises.push_back(std::move(promise));

      if(!socket.is_open())
        socket.open(boost::asio::ip::tcp::v4());

      std::cout << "get_socket connecting" << std::endl;
      
      socket.async_connect
        (endpoint
         , [this] (auto const& ec)
         {
           std::cout << "connected" << std::endl;
           assert(!queued_promises.empty());
           std::cout << "setting value" << std::endl;
           queued_promises[0].set_value(std::move(socket));
           std::cout << "set value" << std::endl;
           queued_promises.pop_front();
           std::cout << "pop front" << std::endl;
         });
      
      std::cout << "returning get_socket()" << std::endl;
      return future;
    }
  }

  void release_socket(boost::asio::ip::tcp::socket /*old_socket*/, boost::system::error_code const& ec)
  {
    std::cout << "release socket" << std::endl;
    if(!queued_promises.empty())
    {
      std::cout << "release socket: there's queue, connecting" << std::endl;
      if(!socket.is_open())
        socket.open(boost::asio::ip::tcp::v4());
      socket.async_connect
        (endpoint
         , [this] (auto const& ec)
         {
           assert(!queued_promises.empty());
           queued_promises[0].set_value(std::move(socket));
           std::cout << "release socket set value 1" << std::endl;
           queued_promises.pop_front();
           std::cout << "release socket pop front 1" << std::endl;
         });
    }
    else
      connected = false;
  }

  boost::asio::ip::tcp::endpoint endpoint;
  boost::asio::ip::tcp::socket socket;
  bool connected;
  std::deque<boost::promise<boost::asio::ip::tcp::socket>> queued_promises;
};

} } }

#endif
