// Copyright Felipe Magno de Almeida 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_DEVICES_HARMONY_DEVICE_IP_HPP
#define EXPERT_DEVICES_HARMONY_DEVICE_IP_HPP

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <boost/asio/ssl/impl/context.ipp>
#include <boost/asio/ssl/impl/error.ipp>
#include <boost/asio/ssl/detail/impl/engine.ipp>
#include <boost/asio/ssl/detail/impl/openssl_init.ipp>
#include <boost/asio/ssl/impl/rfc2818_verification.ipp>

#include <http-parsers/http/request_line.hpp>
#include <http-parsers/http/header.hpp>
#include <http-parsers/http/status_line.hpp>
#include <json-parser/json.hpp>

#include <expert-home/argument_variant.hpp>
#include <functional>

namespace eh { namespace device {

struct harmony_device
{
  struct ssl_state
  {
    boost::asio::ssl::context ssl_context;
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> auth_socket;

    ssl_state(boost::asio::io_service& service)
      : ssl_context(boost::asio::ssl::context::sslv23)
      , auth_socket(service, ssl_context)
    {
    }
  };

  harmony_device(boost::asio::io_service& service
                 , std::string hub_hostname, std::string device
                 , std::string email, std::string password)
    : ssl_state_(new ssl_state{service})
    , hub_socket(service), hub_hostname(hub_hostname), device(device)
    , email(email), password(password)
  {
    ssl_state_->ssl_context.set_default_verify_paths();

    boost::asio::ip::tcp::resolver resolver(ssl_state_->auth_socket.next_layer().get_io_service());
    {
      boost::asio::ip::tcp::resolver::iterator
        iterator = resolver.resolve(boost::asio::ip::tcp::resolver::query{"svcs.myharmony.com", "https"});
      if(iterator != boost::asio::ip::tcp::resolver::iterator())
        auth_endpoint = *iterator;
      else
        throw std::runtime_error("");
    }
  }

  void watch(std::function<void(std::string, std::vector<argument_variant>)> callback)
  {
    // if(!socket.is_open()) socket.open(boost::asio::ip::tcp::v4());
    ssl_state_->auth_socket.lowest_layer().connect(auth_endpoint);
    ssl_state_->auth_socket.lowest_layer().set_option(boost::asio::ip::tcp::no_delay(true));
    
    ssl_state_->ssl_context.set_verify_mode(boost::asio::ssl::verify_peer);
    ssl_state_->ssl_context.set_verify_callback(boost::asio::ssl::rfc2818_verification("myharmony.com"));
    //ssl_state_->ssl_context.load_verify_file("privkey.pem");
    
    //ssl_state_->auth_socket.set_verify_mode(boost::asio::ssl::verify_none);
    ssl_state_->auth_socket.handshake(boost::asio::ssl::stream_base::client);
    
    std::vector<char> request;
    request.reserve(1024);

    namespace x3 = boost::spirit::x3;
    namespace fusion = boost::fusion;
    // svcs.myharmony.com
    std::string body = "{\"password\":\"elF19le\",\"email\": \"felipe.m.almeida@gmail.com\"}";
    std::string length = " ";
    generate(std::back_insert_iterator<std::string>(length)
             , x3::int_
             , body.size());
    std::cout << "length " << length << std::endl << "|" << std::endl;;
    fusion::vector5<const char*, const char*, char, char, std::vector<fusion::vector2<const char*, const char*>>> attr
      ("POST", "/CompositeSecurityServices/Security.svc/json/GetUserAuthToken"
       , '1', '1'
       , {{"Host", " svcs.myharmony.com"}
        , {"Content-Type", " application/json"}
        , {"charset", " utf-8"}
        , {"User-Agent", " curl/7.42.1"}
        , {"Accept", " */*"}
        , {"Content-Length", length.c_str()}
      });
    if(x3::generate(std::back_insert_iterator<std::vector<char>>(request)
                    , http_parsers::http::request_line >> *http_parsers::http::header
                    >> x3::omit["\r\n"] >> x3::omit[x3::string(body)]
                    , attr))
      {
        std::cout << "POST generated ";
        std::copy(request.begin(), request.end(), std::ostream_iterator<char>(std::cout));
        std::endl(std::cout);

        boost::system::error_code ec;
        boost::asio::write(ssl_state_->auth_socket, boost::asio::const_buffers_1(&request[0], request.size()), ec);

        if(ec)
        {
          std::cout << "error sending  "<< ec<< std::endl;
        }
        
        boost::asio::async_read(ssl_state_->auth_socket, boost::asio::mutable_buffers_1(buffer.begin(), buffer.size())
                                , [this] (boost::system::error_code const& ec, std::size_t size)
                                {
                                  std::cout << "error? " << ec << " size " << size << std::endl;
                                  return size != 0;
                                }
                                , [this] (boost::system::error_code const& ec, std::size_t size)
                                {
                                  if(size)
                                  {
                                    std::cout << "received " << size << std::endl;
                                    this->auth_handle(size);
                                  }
                                  else if(ec)
                                  {
                                    std::cout << "error size " << size << " error " << ec.message() << std::endl;
                                  }
                                }
                                );
        

      }
    else
      {
        std::cout << "failed generation |";
        std::copy(buffer.begin(), buffer.end(), std::ostream_iterator<char>(std::cout));
        std::endl(std::cout);
      }
    
    this->callback = callback;
  }

  void change_input(std::string const& input)
  {
  }

  void auth_handle(std::size_t size)
  {
    std::cout << "handling ";
    std::copy(buffer.begin(), buffer.begin() + size, std::ostream_iterator<char>(std::cout));
    std::cout << std::endl;

    namespace fusion = boost::fusion;
    namespace x3 = boost::spirit::x3;

    fusion::vector6<char, char, int, x3::unused_type
                    , std::vector<x3::unused_type>
                           , std::vector<fusion::vector2<std::string, std::string>>> attr;
    auto iterator = buffer.begin();

    auto grammar = http_parsers::http::status_line
                 >> *http_parsers::http::header
                 >> x3::omit["\r\n"]
                 >> x3::omit['{'] >> x3::omit[json_parser::identifier] >> x3::omit[':'] >> x3::omit['{']
                 >> ((json_parser::identifier >> x3::omit[':'] >> json_parser::identifier) % ',')
                 >> x3::omit['}'] >> x3::omit['}']
      ;
    
    if(x3::parse(iterator, buffer.begin() + size
                 , grammar
                 , attr))
    {
      if(fusion::at_c<2>(attr) == 200)
      {
        std::cout << "OK" << std::endl;
        std::copy(iterator, buffer.begin() + size, std::ostream_iterator<char>(std::cout));
        std::cout << std::endl;

        for(auto& pair : fusion::at_c<5>(attr))
          {
            std::cout << fusion::at_c<0>(pair) << ':' << fusion::at_c<1>(pair) << std::endl;
          }
      }
      else
        std::cout << "error?" << std::endl;
    }
    else
      std::cout << "no parse" << std::endl;
  }


  std::array<char, 4096> buffer;
  boost::shared_ptr<struct ssl_state> ssl_state_;
  boost::asio::ip::tcp::socket hub_socket;
  std::string hub_hostname, device, email, password;
  std::function<void(std::string, std::vector<argument_variant>)> callback;
  boost::asio::ip::tcp::endpoint auth_endpoint;
};
    
} }

#endif
