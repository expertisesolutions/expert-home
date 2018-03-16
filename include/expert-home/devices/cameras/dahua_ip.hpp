// Copyright Felipe Magno de Almeida 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_DEVICES_CAMERAS_DAHUA_IP_HPP
#define EXPERT_DEVICES_CAMERAS_DAHUA_IP_HPP

#include <expert-home/devices/cameras/camera_base.hpp>
#include <expert-home/connection_pool.hpp>
#include <expert-home/future/yield_context.hpp>
#include <beast/core.hpp>
#include <beast/http.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/asio/spawn.hpp>
#include <httpc/stateful_connection.hpp>
#include <httpc/md5.hpp>
#include <functional>

#include <fstream>

namespace eh { namespace device { namespace camera {

inline connection_pool& get_connection_pool(boost::asio::ip::tcp::endpoint endpoint, boost::asio::io_service& service)
{
  static std::map<boost::asio::ip::tcp::endpoint, connection_pool> singleton_pool;
  auto iter = singleton_pool.find(endpoint);
  if(iter == singleton_pool.end())
  {
    auto insert_r = singleton_pool.insert(std::make_pair(endpoint, connection_pool{endpoint, service}));
    return insert_r.first->second;
  }
  else
    return iter->second;
}

inline httpc::stateful_connection& get_httpc(boost::asio::ip::tcp::endpoint endpoint, std::string username)
{
  static std::map<std::pair<boost::asio::ip::tcp::endpoint, std::string>, httpc::stateful_connection>
    singleton_pool;
  auto iter = singleton_pool.find(std::make_pair(endpoint, username));
  if(iter == singleton_pool.end())
  {
    auto insert_r = singleton_pool.insert(std::make_pair(std::make_pair(endpoint, username)
                                                         , httpc::stateful_connection{}));
    return insert_r.first->second;
  }
  else
    return iter->second;
}

struct dahua_ip : camera_base
{
  using endpoint_type = boost::asio::ip::tcp::endpoint;
  using socket_type  = boost::asio::ip::tcp::socket;
  using error_code = boost::system::error_code;
  using req_type = beast::http::request<beast::http::string_body>;
  using resp_type = beast::http::response<beast::http::string_body>;

  dahua_ip(endpoint_type endpoint
           , std::string username
           , std::string password
           , int channel
           , boost::asio::io_service& io_service)
    : username(username)
    , password(password)
    , channel(channel)
    , strand(io_service)
    , connection_pool(&get_connection_pool(endpoint, io_service))
    // , httpc(get_httpc(endpoint, username))
  {
  }

  boost::unique_future<snapshot_image> snapshot() const
  {
    std::cout << "snapshot" << std::endl;

    boost::promise<snapshot_image> promise;
    boost::unique_future<snapshot_image> future =  promise.get_future();

    if(busy)
    {
      queued_promises.push_back(std::move(promise));
      std::cout << "queued_promises size " << queued_promises.size() << std::endl;
    }
    else
      run_snapshot(std::move(promise));

    return future;
  }

  void run_snapshot(boost::promise<snapshot_image> promise) const
  {
    std::cout << "run snapshot" << std::endl;
    auto download = [this, promise{std::move(promise)}] (boost::asio::yield_context yield) mutable
      {
        int tries = 0;
        std::cout << "getting connection" << std::endl;


        std::cout << "got socket" << std::endl;

        boost::system::error_code ec;

        resp_type resp;
        do
        {
          ++tries;

          std::cout << "Connecting " << tries << std::endl;
          boost::asio::ip::tcp::socket socket
            = future::async_then(connection_pool->get_socket(), yield);
          std::cout << "Connected! " << tries << std::endl;
          
          std::cout << "sending request, retrying " << tries << std::endl;
          req_type req {beast::http::verb::get, "/cgi-bin/snapshot.cgi?channel=" + std::to_string(channel), 11};
          // req.header["Host"] = "127.0.0.1";
          std::cout << "sending request" << std::endl << req << std::endl << "--eom--" << std::endl;
          httpc::write(socket, req, httpc, ec);
          if (ec == boost::asio::error::connection_reset
              || ec == boost::asio::error::broken_pipe)
          {
            std::cout << "error sending request but is not connected, reconnecting" << std::endl;
            // socket = future::async_then(connection_pool->get_socket(), yield);
          }
          else
          {
            std::cout << "wrote, reading http response" << std::endl;
            beast::multi_buffer sb;
            httpc::async_read(socket, sb, resp, httpc, yield[ec]);

            std::cout << "wrote, reading http response, read " << ec.message()
                      << " resp " << resp.result() << std::endl;
            if(resp.result() == beast::http::status::unauthorized)
            {
              std::cout << "authenticating " << tries << std::endl;
              httpc.authenticate(username, password);
              std::cout << "authenticated? " << tries << std::endl;
            }
            else if(resp.result() == beast::http::status::ok)
            {
              std::cout << "response is ok, breaking out of loop " << tries << std::endl;
              connection_pool->release_socket(std::move(socket), ec);
              break;
            }
            else
            {
              std::cout << "something weird " << resp.result() << std::endl;
              // socket = future::async_then(connection_pool->get_socket(), yield);
            }
          }
          connection_pool->release_socket(std::move(socket), ec);
        }
        while(tries <= 3);

        snapshot_image s;
        if(resp.result() == beast::http::status::ok)
        {
          s.buffer.insert(s.buffer.begin(), resp.body.begin(), resp.body.end());
          s.content_type = "image/jpeg";
          promise.set_value(std::move(s));
          std::cout << "success receiving image size: " << resp.body.size() << std::endl;
        }
        else
        {
          std::cout << "failed receiving any data " << resp.result() << std::endl;
          promise.set_exception(boost::system::system_error(ec));
        }

        unbusy();
      };

    busy = true;
    std::cout << "now busy, starting co-routine" << std::endl;
    boost::asio::spawn(strand, std::move(download));
  }

  void unbusy() const
  {
    std::cout << "un-busying, ended co-routine, promises still waiting: " << queued_promises.size() << std::endl;
    if(!queued_promises.empty())
    {
      std::cout << "there are queued promises" << std::endl;
      auto p = std::move(queued_promises.back());
      queued_promises.pop_back();
      run_snapshot(std::move(p));
    }
    else
      busy = false;
  }

  std::string username;
  std::string password;
  int channel;
  mutable boost::asio::io_service::strand strand;

  mutable bool busy = 0;
  mutable std::vector<boost::promise<snapshot_image> > queued_promises;
  struct connection_pool* connection_pool;
  mutable httpc::stateful_connection httpc;
};
      
} } }

#endif
