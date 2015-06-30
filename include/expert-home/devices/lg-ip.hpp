// Copyright Felipe Magno de Almeida 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_DEVICES_LG_IP_HPP
#define EXPERT_DEVICES_LG_IP_HPP

#include <boost/asio.hpp>

#include <functional>
#include <sstream>

// #include <boost/spirit/home/x3.hpp>
// #include <boost/fusion/include/vector.hpp>
// //#include <boost/spirit/home/support/extended_variant.hpp>
// #include <expert-home/argument_variant.hpp>

namespace eh { namespace device {


// POST /udap/api/pairing HTTP/1.1
// Host: 192.168.33.54:8080
// Cache-Control: no-cache
// Content-Type: text/xml; charset=utf-8
// User-Agent: UDAP/2.0

// GET /udap/api/pairing HTTP/1.1
// Host: 192.168.33.54:8080
// Cache-Control: no-cache
// Content-Type: text/xml; charset=utf-8
// User-Agent: UDAP/2.0


// GET /hdcp/api/data?target=version_info HTTP/1.1
// Host: 192.168.33.54:8080
// Cache-Control: no-cache
// Content-Type: text/xml; charset=utf-8
// User-Agent: UDAP/2.0


// POST /hdcp/api/auth HTTP/1.1
// Content-Type: application/atom+xml
// Content-Length: 74
// Host: 192.168.33.54:8080
// Connection: Keep-Alive

// <?xml version="1.0" encoding="utf-8"?><auth><type>AuthKeyReq</type></auth>

// POST /hdcp/api/auth HTTP/1.1
// Content-Type: application/atom+xml
// Content-Length: 92
// Host: 192.168.33.54:8080
// Connection: Keep-Alive

// <?xml version="1.0" encoding="utf-8"?><auth><type>AuthReq</type><value>SMBDJA</value></auth>

// SMBDJA


// GET /hdcp/api/data?target=context_ui&session=943952412 HTTP/1.1
// Host: 192.168.33.54:8080
// Connection: Keep-Alive
// w


// HTTP/1.1 200 OK
// Date: Mon Jun 29 01:47:46 2015 GMT
// Server: LG HDCP Server
// Pragma: no-cache
// Cache-Control: no-store, no-cache, must-revalidate
// Connection: close
// Content-Length: 150
// Content-Type: application/atom+xml; charset=utf-8

// <?xml version="1.0" encoding="utf-8"?><envelope><HDCPError>200</HDCPError><HDCPErrorDetail>OK</HDCPErrorDetail><session>943952412</

// POST /hdcp/api/dtv_wifirc HTTP/1.1
// Host: 192.168.33.54:8080
// Content-Type: application/atom+xml
// Content-Length: 129
// Connection: Keep-Alive

// <?xml version="1.0" encoding="utf-8"?><command><session>1535748147</session><type>HandleKeyInput</type><value>1</value></command>

    
struct lg_ip
{
  lg_ip(boost::asio::io_service& service, std::string hostname, std::string password)
    : socket(service), watch_socket(service), hostname(hostname), password(password)
  {
    boost::asio::ip::tcp::resolver resolver(watch_socket.get_io_service());
    {
      boost::asio::ip::tcp::resolver::iterator
        iterator = resolver.resolve(boost::asio::ip::tcp::resolver::query{hostname, "8080"});
      if(iterator != boost::asio::ip::tcp::resolver::iterator())
        endpoint = *iterator;
      else
        throw std::runtime_error("");
    }
  }

  void change_input(std::string const& input)
  {
    queue_command(input);
  }

  void queue_command(std::string const& input)
  {
    std::cout << "queue command " << input << std::endl;
    
    socket.connect(endpoint);

    std::stringstream body;
    body << "<?xml version=\"1.0\" encoding=\"utf-8\"?><command><session>"
         << session
         << "</session><type>HandleKeyInput</type><value>"
         << input
         << "</value></command>";
    std::string body_string = body.str();
    std::stringstream message;
    message <<
      "POST /hdcp/api/dtv_wifirc HTTP/1.1\r\n"
      "Host: 192.168.33.54:8080\r\n"
      "Content-Type: application/atom+xml\r\n"
      "Content-Length: " << body_string.size() <<
      "\r\nConnection: Keep-Alive\r\n\r\n" <<
      body_string
      ;

    std::string request = message.str();
    boost::asio::write(socket, boost::asio::const_buffers_1(&request[0], request.size())
                       // , [this] (boost::system::error_code const& ec, std::size_t size)
                       // {
                       //   std::cout << "sent I think" << std::endl;
                       // }
                       );
  }
  
  void watch(std::function<void(std::string, std::vector<argument_variant>)> function)
  {
    socket.connect(endpoint);

    std::string body = "<?xml version=\"1.0\" encoding=\"utf-8\"?><auth><type>AuthReq</type><value>"
      + password + "</value></auth>";
    std::stringstream message;
    message <<
      "POST /hdcp/api/auth HTTP/1.1\r\n"
      "Content-Type: application/atom+xml\r\n"
      "Content-Length: " << body.size() << "\r\n" <<
      "Host: 192.168.33.54:8080\r\n" <<
      "Connection: Keep-Alive\r\n\r\n" << body;

    boost::system::error_code ec;
    
    std::string request = message.str();
    boost::asio::write(socket, boost::asio::const_buffers_1(&request[0], request.size())
                       // , [this] (boost::system::error_code const& ec, std::size_t size)
                       // {
                       //   std::cout << "sent I think" << std::endl;
                       // }
                       , ec
                       );

    boost::asio::async_read(socket, boost::asio::mutable_buffers_1(buffer.begin(), buffer.size())
                            // , [this] (boost::system::error_code const& ec, std::size_t size)
                            // {
                            //   auto last = buffer.begin() + size;
                            //   return ec || size == 1024 || std::find(buffer.begin(), last, '\r') != last;
                            // }
                            // , boost::bind(&denon_ip::handler, this, _1, _2)
                            , [this] (boost::system::error_code const& ec, std::size_t size)
                            {
                              // auto last = buffer.begin() + size;
                              // return ec || size == 1024 || std::find(buffer.begin(), last, '\r') != last;
                              if(size)
                              {
                                std::cout << "received " << size << std::endl;
                                this->handle(size);
                              }
                              else if(ec)
                              {
                                std::cout << "error size " << size << " error " << ec.message() << std::endl;
                              }
                            }
                            );


    this->function = std::move(function);
  }

  void handle(std::size_t size)
  {
    namespace x3 = boost::spirit::x3;
    namespace fusion = boost::fusion;
    auto iterator = buffer.begin();
    boost::fusion::vector3<int, std::string, unsigned int> attr;
    if(x3::parse(iterator, buffer.begin() + size
                 , x3::omit[+(x3::char_ - ("\r\n\r\n"))]
                 >> "\r\n\r\n"
                 >> "<?xml version=\"1.0\" encoding=\"utf-8\"?><envelope><HDCPError>"
                 >> x3::int_
                 >> "</HDCPError><HDCPErrorDetail>"
                 >> (+(x3::char_ - '<'))
                 >> "</HDCPErrorDetail><session>"
                 >> x3::uint_
                 >> "</session></envelope>"
                 , attr))
    {
      session = fusion::at_c<2>(attr);
      std::cout << "LG Session " << session << std::endl;
    }
    else
    {
      std::cout << "error parsing" << std::endl;
    }
    socket.close();
  }
  
  std::array<char, 4096> buffer;
  boost::asio::ip::tcp::socket socket;
  boost::asio::ip::tcp::socket watch_socket;
  std::string hostname;
  std::string password;
  std::vector<std::string> commands;
  boost::asio::ip::tcp::endpoint endpoint;
  unsigned int session;
  std::function<void(std::string, std::vector<argument_variant>)> function;
};

} }

#endif

