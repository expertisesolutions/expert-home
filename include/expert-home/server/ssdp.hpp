// Copyright Felipe Magno de Almeida 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_SERVER_SSDP_HPP
#define EXPERT_SERVER_SSDP_HPP

#include <boost/asio.hpp>

#include <http-parsers/http/request_line.hpp>
#include <http-parsers/http/header.hpp>
#include <http-parsers/http/status_line.hpp>
#include <json-parser/json.hpp>

namespace eh { namespace server {

namespace ssdp_detail {

struct ssdp_task
{
  boost::asio::ip::udp::endpoint remote_endpoint;
  std::array<char, 4096> buffer;
};
  
void asynchronous_ssdp(boost::asio::io_service* service, std::vector<std::string> responses
                       , boost::asio::ip::udp::socket* ssdp_socket)
{
  namespace x3 = boost::spirit::x3;
  
  boost::shared_ptr<struct ssdp_task> ssdp_task
    (new struct ssdp_task);
  ssdp_socket->async_receive_from
    (boost::asio::mutable_buffers_1(&ssdp_task->buffer[0], ssdp_task->buffer.size())
     , ssdp_task->remote_endpoint
     // , [ssdp_task] (boost::system::error_code const& ec, std::size_t size)
     // {
     //   return x3::parse(ssdp_task->buffer.begin(), ssdp_task->buffer.begin() + size
     //                    , x3::lit("M-SEARCH * HTTP/1.1\r\n")
     //                    >> *http_parsers::http::header
     //                    >> x3::lit("\r\n")
     //                    );
     // }
     , [=] (boost::system::error_code const& ec, std::size_t size)
     {
       std::cout << "received ssdp packet" << std::endl;
       // std::copy(ssdp_task->buffer.begin(), ssdp_task->buffer.begin() + size, std::ostream_iterator<char>(std::cout));
       // std::endl(std::cout);

       if(x3::parse(ssdp_task->buffer.begin(), ssdp_task->buffer.begin() + size
                    , x3::lit("M-SEARCH * HTTP/1.1\r\n")))
       {
         std::cout << "sending response" << std::endl;
         for(auto& response : responses)
           ssdp_socket->send_to
             (boost::asio::const_buffers_1(&response[0], response.size())
              , ssdp_task->remote_endpoint);
       }

       asynchronous_ssdp(service, responses, ssdp_socket);
     }
     );
}

}

struct ssdp
{
  ssdp(boost::asio::io_service& service, unsigned short port)
    : socket(service, boost::asio::ip::udp::v4())
  {
    std::cout << "ssdp server" << std::endl;;
    socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
    std::cout << __LINE__ << std::endl;;
    socket.set_option(boost::asio::ip::multicast::join_group
                           (boost::asio::ip::address::from_string("239.255.255.250")));
    std::cout << "going to listen for SSDP" << std::endl;;
    socket.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), port));
    std::cout << "listening for SSDP" << std::endl;;

    ssdp_detail::asynchronous_ssdp(&service, responses, &socket);
  }

  void add_answer(std::string&& response)
  {
    responses.push_back(std::move(response));
  }

  boost::asio::ip::udp::socket socket;
  std::vector<std::string> responses;
};
  
} }
    
#endif
