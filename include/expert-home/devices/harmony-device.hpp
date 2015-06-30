// Copyright Felipe Magno de Almeida 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_DEVICES_HARMONY_DEVICE_IP_HPP
#define EXPERT_DEVICES_HARMONY_DEVICE_IP_HPP

#include <boost/asio.hpp>

#include <http-parsers/http/request_line.hpp>
#include <http-parsers/http/header.hpp>

#include <expert-home/argument_variant.hpp>
#include <functional>

namespace eh { namespace device {

struct harmony_device
{
  harmony_device(boost::asio::io_service& service
                 , std::string hub_hostname, std::string device
                 , std::string email, std::string password)
    : socket(service), hub_hostname(hub_hostname), device(device)
    , email(email), password(password)
  {
    // curl -X POST -H "Content-Type: application/json" -H "charset: utf-8" -d '{"password":"elF19le","email": "felipe.m.almeida@gmail.com"}' https://svcs.myharmony.com/CompositeSecurityServices/Security.svc/json/GetUserAuthToken
  }

  void watch(std::function<void(std::string, std::vector<argument_variant>)> callback)
  {
    
    
    this->callback = callback;
  }

  void change_input(std::string const& input)
  {
    if(!socket.is_open()) socket.open(boost::asio::ip::tcp::v4());

    std::vector<char> buffer;
    buffer.reserve(1024);

    namespace x3 = boost::spirit::x3;
    namespace fusion = boost::fusion;
    // svcs.myharmony.com
    fusion::vector5<const char*, const char*, char, char, std::vector<fusion::vector2<const char*, const char*>>> attr
      ("POST", "/CompositeSecurityServices/Security.svc/json/GetUserAuthToken"
       , '1', '1'
       , {{"Host", "svcs.myharmony.com"}
        , {"Content-Type", "application/json"}
        , {"charset", "utf-8"}
      });
    if(x3::generate(std::back_insert_iterator<std::vector<char>>(buffer)
                    , http_parsers::http::request_line >> *http_parsers::http::header
                    >> content_length
                    [
                     x3::omit["\r\n"] >> x3::omit["{\"password\":\"elF19le\",\"email\": \"felipe.m.almeida@gmail.com\"}"]
                    ]
                    , attr))
      {
        std::cout << "POST generated ";
        std::copy(buffer.begin(), buffer.end(), std::ostream_iterator<char>(std::cout));
        std::endl(std::cout);
      }
    else
      {
        std::cout << "failed generation |";
        std::copy(buffer.begin(), buffer.end(), std::ostream_iterator<char>(std::cout));
        std::endl(std::cout);
      }
    
  }
  
  boost::asio::ip::tcp::socket socket;
  std::string hub_hostname, device, email, password;
  std::function<void(std::string, std::vector<argument_variant>)> callback;
};
    
} }

#endif
