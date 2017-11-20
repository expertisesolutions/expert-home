// Copyright Felipe Magno de Almeida 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_DEVICES_CAMERAS_DAHUA_IP_HPP
#define EXPERT_DEVICES_CAMERAS_DAHUA_IP_HPP

#include <expert-home/devices/cameras/camera_base.hpp>
#include <beast/core.hpp>
#include <beast/http.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/asio/spawn.hpp>
#include <httpc/stateful_connection.hpp>
#include <httpc/md5.hpp>
#include <functional>

#include <fstream>

namespace eh { namespace device { namespace camera {

struct dahua_ip : camera_base
{
  using endpoint_type = boost::asio::ip::tcp::endpoint;
  using socket_type  = boost::asio::ip::tcp::socket;
  using error_code = boost::system::error_code;
  using req_type = beast::http::request<beast::http::string_body>;
  using resp_type = beast::http::response<beast::http::string_body>;

  dahua_ip(endpoint_type endpoint, boost::asio::io_service& io_service)
    : endpoint(endpoint)
    , socket(io_service)
    , strand(io_service)
  {
    socket.open(boost::asio::ip::tcp::v4());
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
    auto download = [this, promise{std::move(promise)}] (boost::asio::yield_context yield) mutable
      {
        resp_type resp;
        int tries = 0;
        boost::system::error_code ec;

        // boost::scoped_lock<boost::mutex> lock(mutex);
        // if(        
        
        std::cout << "trying connection" << std::endl;

        do
        {
          resp = {};
          resp.result(beast::http::status::unknown);
          ec = {};

          std::cout << "resp is initialized with " << resp.result() << std::endl;
          
          if(!socket.is_open())
          {
            socket.close();
            socket.open(boost::asio::ip::tcp::v4());
            socket.async_connect(endpoint, yield[ec]);
            if(ec) continue;
          }
          
          ++tries;
          req_type req {beast::http::verb::get, "/cgi-bin/snapshot.cgi", 11};
          // req.header["Host"] = "127.0.0.1";
          std::cout << "sending request" << std::endl << req << std::endl << "--eom--" << std::endl;
          httpc::write(socket, req, httpc, ec);
          if (ec == boost::asio::error::connection_reset
              || ec == boost::asio::error::broken_pipe)
          {
            std::cout << "error sending request but is not connected, reconnecting" << std::endl;
            socket.close();
            socket.open(boost::asio::ip::tcp::v4());
            socket.async_connect(endpoint, yield[ec]);
            continue;
          }
          else if(ec)
          {
            std::cout << "error sending request " << ec.message() << std::endl;
            //return promise.set_exception(boost::system::system_error(ec));
            continue;
          }

          beast::multi_buffer sb;
          httpc::async_read(socket, sb, resp, httpc, yield[ec]);
          if(ec) continue;
          // std::cout << "response" << std::endl << resp << std::endl << "--eom--" << std::endl;
          if(resp.result() == beast::http::status::unauthorized)
          {
            httpc.authenticate("admin", "admin");
          }
          else if(resp.result() == beast::http::status::ok)
          {
            std::cout << "response is ok" << std::endl;
          }
          else
          {
            std::cout << "something weird " << resp.result() << std::endl;
          }
          // socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        }
        while((
               resp.result() == beast::http::status::unauthorized
               || resp.result() == beast::http::status::unknown
              )
              && tries != 6);
      
        // std::cout << "response" << std::endl << resp << std::endl << "--eom--" << std::endl;

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
          std::cout << "failed receiving any data" << std::endl;
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
    std::cout << "un-busying, ended co-routine" << std::endl;
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

  endpoint_type endpoint;
  mutable socket_type socket;
  boost::asio::io_service::strand strand;
  mutable httpc::stateful_connection httpc;

  mutable bool busy = 0;
  mutable std::vector<boost::promise<snapshot_image> > queued_promises;
};
      
} } }

#endif
