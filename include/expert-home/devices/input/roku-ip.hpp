// Copyright Felipe Magno de Almeida 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_DEVICES_INPUT_ROKU_IP_HPP
#define EXPERT_DEVICES_INPUT_ROKU_IP_HPP

#include <boost/asio.hpp>

#include <http-parsers/http/request_line.hpp>
#include <http-parsers/http/header.hpp>
#include <http-parsers/http/status_line.hpp>
#include <json-parser/json.hpp>

#include <expert-home/server/ssdp.hpp>

namespace eh { namespace device {

namespace roku_detail {
  
const char answer_dlna[] =
  "HTTP/1.1 200 OK\r\n"
  "Content-Length: 1906\r\n"
  "Content-Type: text/xml; charset=\"utf-8\"\r\n"
  "Connection: close\r\n"
  "\r\n"
  "<?xml version=\"1.0\"?>\r\n"
  "<root xmlns=\"urn:schemas-upnp-org:device-1-0\" xmlns:ms=\"urn:microsoft-com:wmc-1-0\"> >\r\n"
  "<specVersion>\r\n"
  "<major>1</major>\r\n"
  "<minor>0</minor>\r\n"
  "</specVersion>\r\n"
  "<device ms:X_MS_SupportsWMDRM=\"true\">\r\n"
  "<dlna:X_DLNADOC xmlns:dlna=\"urn:schemas-dlna-org:device-1-0\">DMP-1.00</dlna:X_DLNADOC>\r\n"
  "<deviceType>urn:schemas-upnp-org:device:MediaRenderer:1</deviceType>\r\n"
  "<friendlyName>Roku Streaming Player</friendlyName>\r\n"
  "<manufacturer>Roku</manufacturer>\r\n"
  "<manufacturerURL>http://www.roku.com/</manufacturerURL>\r\n"
  "<modelDescription>Roku Streaming Player Network Media</modelDescription>\r\n"
  "<modelName>Roku Streaming Player 2050X</modelName>\r\n"
  "<modelNumber>2050X</modelNumber>\r\n"
  "<modelURL>http://www.roku.com/</modelURL>\r\n"
  "<serialNumber>P0A070000007</serialNumber>\r\n"
  "<UDN>uuid:roku:ecp:P0A070000007</UDN>\r\n"
  "<UPC></UPC>\r\n"
  "<serviceList>\r\n"
  "<service>\r\n"
  "<serviceType>urn:schemas-upnp-org:service:RenderingControl:1</serviceType>\r\n"
  "<serviceId>urn:upnp-org:serviceId:RenderingControl</serviceId>\r\n"
  "<SCPDURL>/RenderCtl.xml</SCPDURL>\r\n"
  "<controlURL>/UD/?0</controlURL>\r\n"
  "<eventSubURL>/?0</eventSubURL>\r\n"
  "</service>\r\n"
  "<service>\r\n"
  "<serviceType>urn:schemas-upnp-org:service:ConnectionManager:1</serviceType>\r\n"
  "<serviceId>urn:upnp-org:serviceId:ConnectionManager</serviceId>\r\n"
  "<SCPDURL>/ConnMgr.xml</SCPDURL>\r\n"
  "<controlURL>/UD/?1</controlURL>\r\n"
  "<eventSubURL>/?1</eventSubURL>\r\n"
  "</service>\r\n"
  "<service>\r\n"
  "<serviceType>urn:schemas-upnp-org:service:AVTransport:1</serviceType>\r\n"
  "<serviceId>urn:upnp-org:serviceId:AVTransport</serviceId>\r\n"
  "<SCPDURL>/AvTransport.xml</SCPDURL>\r\n"
  "<controlURL>/UD/?2</controlURL>\r\n"
  "<eventSubURL>/?2</eventSubURL>\r\n"
  "</service>\r\n"
  "<service>\r\n"
  "<serviceType>urn:roku-com:service:ecp:1</serviceType>\r\n"
  "<serviceId>urn:roku-com:serviceId:ecp1.0</serviceId>\r\n"
  "<controlURL></controlURL>\r\n"
  "<eventSubURL></eventSubURL>\r\n"
  "<SCPDURL>ecp_SCPD.xml</SCPDURL>\r\n"
  "</service>\r\n"
  "</serviceList>\r\n"
  "<presentationURL>/</presentationURL>\r\n"
  "</device>\r\n"
  "</root>\r\n"
  ;

struct http_task
{
  boost::asio::ip::tcp::socket socket;
  std::array<char, 1500> buffer;
  std::function<void(std::string, std::vector<argument_variant>)> callback;

  http_task(boost::asio::io_service& service
            , std::function<void(std::string, std::vector<argument_variant>)> callback)
    : socket(service), callback(callback)
  {
  }
};

void asynchronous_http_read(boost::shared_ptr<struct http_task> http_task);
  
void asynchronous_http_read_handler(boost::system::error_code const& ec, std::size_t size
                                    , boost::shared_ptr<struct http_task> http_task)
{
  std::cout << "asynchronous_http_read_handler size " << size << " ec " << ec << std::endl;
  if(size)
  {
    namespace pl = std::placeholders;
    std::cout << "read " << size << std::endl;

    std::copy(http_task->buffer.begin(), http_task->buffer.begin() + size
              , std::ostream_iterator<char>(std::cout));
    std::cout << std::endl;

    namespace x3 = boost::spirit::x3;
    namespace fusion = boost::fusion;
    
    fusion::vector5<std::string, std::string, char, char, std::vector<fusion::vector2<std::string, std::string>>> attr;
    if(x3::parse(http_task->buffer.begin(), http_task->buffer.begin() + size
                 , http_parsers::http::request_line
                 >> *http_parsers::http::header
                 >> x3::omit["\r\n"]
                 , attr))
    {
      if(fusion::at_c<1>(attr) == "/")
      {
        std::cout << "Send DLNA profile" << std::endl;
        boost::asio::write(http_task->socket, boost::asio::const_buffers_1(&answer_dlna[0], sizeof(answer_dlna)-1));
      }
      else if(fusion::at_c<1>(attr) == "/query/apps")
      {
        std::cout << "Send Roku Apps" << std::endl;
        const char apps[] =
          "<apps>\r\n"
          "<app id=\"1\">TV Sala</app>\r\n"
          "<app id=\"2\">TV Cozinha</app>\r\n"
          "<app id=\"3\">ChromeCast</app>\r\n"
          "<app id=\"4\">Jogos</app>\r\n"
          "<app id=\"5\">Filmes</app>\r\n"
          "<app id=\"12\">Netflix</app>\r\n"
          //"<app id=\"13\">Amazon Video on Demand</app>\r\n"
          //"<app id=\"14\">MLB.TV®</app>\r\n"
          //"<app id=\"26\">Free FrameChannel Service</app>\r\n"
          //"<app id=\"27\">Mediafly</app>\r\n"
          //"<app id=\"28\">Pandora</app>\r\n"
          "</apps>\r\n"
          ;
        
        boost::asio::write(http_task->socket, boost::asio::const_buffers_1(&apps[0], sizeof(apps)-1));
      }
      else
      {
        std::string key;
        int app;
        if(x3::parse(fusion::at_c<1>(attr).begin(), fusion::at_c<1>(attr).end()
                     , x3::lit("/keypress/") >> +x3::char_
                     , key))
        {
          std::cout << "Pressed " << key << std::endl;
          http_task->callback(key, std::vector<argument_variant>{});
        }
        else if(x3::parse(fusion::at_c<1>(attr).begin(), fusion::at_c<1>(attr).end()
                          , x3::lit("/launch/") >> x3::int_
                          , app))
        {
          http_task->callback("launch", std::vector<argument_variant>{app});
        }
        else
        {
          std::cout << "Failed parsing url location" << std::endl;
        }
        asynchronous_http_read(http_task);
    
        const char response[] =
          "HTTP/1.1 200 OK\r\n"
          "Server: Roku UPnP/1.0 MiniUPnPd/1.4\r\n"
          "Content-Length: 0\r\n"
          "connection: Close\r\n"
          "\r\n"
          ;
    
        boost::asio::write(http_task->socket, boost::asio::const_buffers_1(&response[0], sizeof(response)-1));
      }
    }
    else
      std::cout << "Failed parsing ??" << std::endl;
  }
  else if(ec)
  {
    std::cout << "Error " << ec.message() << std::endl;
  }
}

void asynchronous_http_read(boost::shared_ptr<struct http_task> http_task)
{
  // std::cout << "asynchronous_http_read" << std::endl;
  namespace pl = std::placeholders;
  boost::asio::async_read
    (http_task->socket
     , boost::asio::mutable_buffers_1(&http_task->buffer[0], http_task->buffer.size())
     , [=] (boost::system::error_code const& ec, std::size_t size)
     {
       namespace x3 = boost::spirit::x3;
       // std::cout << "completion test " << size << " ec " << ec << std::endl;
       auto last = http_task->buffer.begin() + size;
       const char crlfcrlf[] = "\r\n\r\n";
       return ec
         || std::search(http_task->buffer.begin(), last
                        , &crlfcrlf[0], &crlfcrlf[0] + sizeof(crlfcrlf)-1) != last;
         ;
     }
     , std::bind(asynchronous_http_read_handler, pl::_1, pl::_2, http_task));
}
  
void asynchronous_http(boost::asio::ip::tcp::acceptor* acceptor
                       , std::function<void(std::string, std::vector<argument_variant>)> callback)
{
  boost::shared_ptr<struct http_task> http_task
    (new struct http_task(acceptor->get_io_service(), callback));
  acceptor->async_accept
    (http_task->socket
     , [=] (boost::system::error_code const& ec)
     {
       std::cout << "ROKU === Accepted HTTP connection" << std::endl;
       http_task->socket.set_option(boost::asio::ip::tcp::no_delay(true));
       asynchronous_http_read(http_task);
       asynchronous_http(acceptor, http_task->callback);
     }
     );
}

}


struct roku_ip
{
  roku_ip(boost::asio::io_service& service, std::string listen_ip
          , unsigned short port
          , server::ssdp& ssdp_server
          , std::string serial)
    : acceptor(service, boost::asio::ip::tcp::endpoint
               (boost::asio::ip::address::from_string(listen_ip), port))
  {
    namespace x3 = boost::spirit::x3;
    namespace fusion = boost::fusion;

    auto ssdp_response_grammar_def = 
      x3::lit
      ("HTTP/1.1 200 OK\r\n"
       "Cache-Control: max-age=300\r\n"
       "ST: roku:ecp\r\n"
       "Location: http://")
      >> +x3::char_
      >> ':'
      >> x3::int_
      >> x3::lit
      ("/\r\n"
       "USN: uuid:roku:ecp:")
      >> +x3::char_
      >> x3::lit("\r\n\r\n")
      ;

    std::string ssdp_response;
    if(x3::generate(std::back_insert_iterator<std::string>(ssdp_response)
                    , ssdp_response_grammar_def
                    , fusion::vector3<std::string const&, unsigned short, std::string const&>
                    (listen_ip, port, serial)))
    {
      std::cout << "Generated SSDP" << std::endl;
      std::copy(ssdp_response.begin(), ssdp_response.end()
                , std::ostream_iterator<char>(std::cout));
      std::cout << std::endl;
      ssdp_server.add_answer(std::move(ssdp_response));
    }
  }

  void watch(std::function<void(std::string, std::vector<argument_variant>)> callback)
  {
    this->callback = callback;
    roku_detail::asynchronous_http(&acceptor, callback);
  }

  boost::asio::ip::tcp::acceptor acceptor;
  std::function<void(std::string, std::vector<argument_variant>)> callback;
};
  
} }

#endif
