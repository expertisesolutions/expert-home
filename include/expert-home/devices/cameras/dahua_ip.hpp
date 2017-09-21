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
#include <httpc/stateful_connection.hpp>
#include <httpc/md5.hpp>

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
  {
  }

  snapshot_image snapshot() const
  {
    // std::ifstream f("image.jpg");
    // f.seekg(0, std::ios::end);
    // std::size_t size = f.tellg();
    // f.seekg(0, std::ios::beg);
    // s.buffer.resize(size);

    // if(size != 0)
    //   f.rdbuf()->sgetn(&s.buffer[0], size);

    // s.content_type = "image/jpeg";

    std::cout << "snapshot" << std::endl;
    socket_type socket(const_cast<socket_type&>(this->socket).get_io_service());
    snapshot_image s;

    // try
    {
      httpc::stateful_connection httpc;
      resp_type resp;
      int tries = 0;
      std::cout << "trying connection" << std::endl;

      do
      {
        resp = {};
        socket.connect(endpoint);
        ++tries;
        req_type req {beast::http::verb::get, "/cgi-bin/snapshot.cgi" /*"?loginuse=admin&loginpas=elF19le"*/, 11};
        // req.header["Host"] = "127.0.0.1";
        std::cout << "sending request" << std::endl << req << std::endl << "--eom--" << std::endl;
        httpc::write(socket, req, httpc);

        beast::multi_buffer sb;
        httpc::read(socket, sb, resp, httpc);
        std::cout << "response" << std::endl << resp << std::endl << "--eom--" << std::endl;
        if(resp.result() == beast::http::status::unauthorized)
        {
          httpc.authenticate("admin", "admin");
        }
        socket.close();
      }
      while(resp.result() == beast::http::status::unauthorized
            && tries != 3);
      
      // typedef boost::archive::iterators::base64_from_binary<
      //   boost::archive::iterators::transform_width<
      //     const char *
      //     ,6
      //     ,sizeof(char) * 8
      //     >
      //   > iterator_type;

      // std::string credentials = "Basic ";
      // static const char userpass[] = "admin:elF19le";
      // std::copy(iterator_type(&userpass[0])
      //           , iterator_type(&userpass[sizeof(userpass)-1])
      //           , std::back_inserter(credentials));

      // req_type req {beast::http::verb::get, "/cgi-bin/snapshot.cgi" /*"?loginuse=admin&loginpas=elF19le"*/, 11};
      // // req.version = 11;
      // // req.method = "GET";
      // // req.url = "/cgi-bin/snapshot.cgi?loginuse=admin&loginpas=admin";
      // req.set("Authorization", credentials);

      // std::cout << "sending request" << std::endl << req << std::endl << "--eom--" << std::endl;
      // beast::http::write(socket, req);

      // beast::multi_buffer sb;
    
      // resp_type resp;
      // beast::http::read(socket, sb, resp);

      std::cout << "response" << std::endl << resp << std::endl << "--eom--" << std::endl;

      if(resp.result() == beast::http::status::ok)
        {
          s.buffer.insert(s.buffer.begin(), resp.body.begin(), resp.body.end());
          s.content_type = "image/jpeg";
        }
    }
    // catch(boost::exception& e)
    // {
    //   std::cout << "Error connecting" << std::endl;
    // }
    //http://192.168.33.5/cgi-bin/ptz.cgi?action=start&channel=0&code=PositionABS&arg1=0&arg2=80&arg3=0
    return s;
  }

  endpoint_type endpoint;
  socket_type socket;
};
      
} } }

#endif
