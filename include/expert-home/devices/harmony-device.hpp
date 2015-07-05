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

#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/fusion/adapted/std_pair.hpp>

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
    : offset(0), ssl_state_(new ssl_state{service})
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
    {
      boost::asio::ip::tcp::resolver::iterator
        iterator = resolver.resolve(boost::asio::ip::tcp::resolver::query{hub_hostname, "xmpp-client"});
      if(iterator != boost::asio::ip::tcp::resolver::iterator())
        hub_endpoint = *iterator;
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

  static std::string decode64(const std::string &val)
  {
    using namespace boost::archive::iterators;
    using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
    return boost::algorithm::trim_right_copy_if(std::string(It(std::begin(val)), It(std::end(val))), [](char c) {
        return c == '\0';
    });
  }

  static std::string encode64(const std::string &val)
  {
    using namespace boost::archive::iterators;
    using It = base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>;
    auto tmp = std::string(It(std::begin(val)), It(std::end(val)));
    return tmp.append((3 - val.size() % 3) % 3, '=');
  }
  
  void change_input(std::string const& input)
  {
    namespace x3 = boost::spirit::x3;
    namespace fusion = boost::fusion;
    
    auto cmd_grammar_def =
      x3::lit("{\"id\":\"expert-home\",\"cmd\":\"")
      >> +x3::char_ // cmd action
      >> x3::lit("\",\"token\":\"")
      >> +x3::char_ // auth token
      >> x3::lit("\",\"params\":{\"action\":\"{\\\"comamnd\\\":\\\"")
      >> +x3::char_ // cmd name
      >> x3::lit("\\\",\\\"type\\\":\\\"IRCommand\\\",\\\"deviceId\\\":\\\"")
      >> +x3::char_ // deviceId
      >> x3::lit("\\\"}\",\"status\":\"")
      >> +x3::char_ // status (release, press?)
      >> x3::lit("\",\"timestamp\":39067}}")
      ;

    std::string token = encode64("2da85ac6-2999-4019-8a60-a620910f64d3\0");
    
    boost::fusion::vector5<const char*, std::string, const char*, std::string, const char*>
      attr("vnd.logitech.harmony\\/vnd.logitech.harmony.engine?holdAction"
           , token, "PowerOff", device, "release");
    std::vector<char> request;
    if(x3::generate(std::back_insert_iterator<std::vector<char>>(request)
                    , cmd_grammar_def
                    , attr))
      {
        auto message =
          x3::lit("POST / HTTP/1.1\r\n"
                  "Content-Type: application/json\r\n"
                  "charset: utf-8\r\n"
                  "Accept: application/json\r\n"
                  "Content-Length: ")
          >> x3::int_
          >> x3::lit("\r\n"
                     "Origin: http://sl.dhg.myharmony.com\r\n"
                     "\r\n")
          >> +x3::char_
          ;
        std::vector<char> final_request;
        if(x3::generate(std::back_insert_iterator<std::vector<char>>(final_request)
                        , message
                        , fusion::vector2<int, std::vector<char>const&>(request.size(), request)))
        {
          std::cout << "connect" << std::endl;
          std::cout << "connected" << std::endl;

          const char final_request[] =
            "GET /?domain=svcs.myharmony.com&hubId=6764415 HTTP/1.1\r\n"
            "Connection: Upgrade\r\n"
            "Host: 192.168.20.118:8088\r\n"
            "Sec-WebSocket-Key: EZk/CC77fjnnX/FrURBsMA==\n"
            "Sec-WebSocket-Version: 13\r\n"
            "Upgrade: websocket\r\n"
            "\r\n"
            ;
          
          boost::system::error_code ec;
          boost::asio::write(hub_socket, boost::asio::const_buffers_1(&final_request[0], sizeof(final_request)-1), ec);
          std::cout << "written" << std::endl;

          // std::cout << "generated ";
          // std::copy(final_request.begin(), final_request.end(), std::ostream_iterator<char>(std::cout));
          // std::endl(std::cout);

          std::cout << "going to read" << std::endl;
          // std::vector<char> buffer(4096);
          offset = 0;
          boost::asio::async_read(hub_socket, boost::asio::mutable_buffers_1(&buffer[0], buffer.size())
                                  , [this] (boost::system::error_code const& ec, std::size_t size)
                                  {
                                    std::cout << "async read ("<< offset << ',' << size-offset << ")" << std::endl;;
                                    std::copy(buffer.begin() + offset, buffer.begin() + size
                                              , std::ostream_iterator<char>(std::cout));
                                    std::endl(std::cout);
                                    offset = size;
                                    return false;
                                  }
                                  , [] (boost::system::error_code const&, std::size_t) {});
        }
      }
    else
      {
        std::cout << "fail generated" << std::endl;
      }
    
    char cmdcmd0[] = "connect.ping";
    char cmdcmd1[] = "connect.rf?info";
    char cmdcmd2[] = "connect.statedigest?get";
    char cmdcmd3[] = "vnd.logitech.connect/vnd.logitech.deviceinfo?get";
    char cmdcmd4[] = "vnd.logitech.harmony/vnd.logitech.harmony.engine?config";

    //{"hbus":{"id":"616003bc72600062#kltelgt#sm-g900l-296-5","cmd":"vnd.logitech.harmony\/vnd.logitech.harmony.engine?holdAction","token":"WFQIvYIg1rj5l19n0dXfnQ;1434341052;lNB_4xG0qjhH8qnQsVKfd38-RzI","params":{"action":"{\"command\":\"PowerToggle\",\"type\":\"IRCommand\",\"deviceId\":\"27568418\"}","status":"release","timestamp":39067}}}
    
    char cmd2[] =
      "POST / HTTP/1.1\r\n"
      "Content-Type: application/json\r\n"
      "charset: utf-8\r\n"
      "Accept: application/json\r\n"
      "Content-Length: 33\r\n"
      "Origin: http//:localhost.nebula.myharmony.com\r\n"
      "\r\n"
      "{\"id\":\"124\",\"cmd\":\"connect.ping\"}"
      ;

    
    char cmd1[] =
      "POST / HTTP/1.1\r\n"
      "Content-Type: application/json\r\n"
      "charset: utf-8\r\n"
      "Origin: http://sl.dhg.myharmony.com\r\n"
      "Accept: application/json, text/javascript, */*; q=0.01\r\n"
      "Content-Length: 340\r\n"
      "\r\n"
      "{\"hbus\":{\"id\":\"expert-home\",\"cmd\":\"vnd.logitech.harmony/vnd.logitech.harmony.engine?holdAction\",\"token\":\"zzG0OE0CFct5gXwapANmj3z/y5pbHKLDNhtu0gCzX2CP+m8X380cQwSXou7IhkD4\",\"params\":{\"action\":\"{\\\"command\\\":\\\"PowerToggle\\\",\\\"type\\\":\\\"IRCommand\\\",\\\"deviceId\\\":\\\"27568418\\\"}\",\"status\":\"release\",\"timestamp\":39067}}}"
      ;
    
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
                           , std::map<std::string, std::string>> attr;
    auto iterator = buffer.begin();

    static_assert((fusion::traits::is_sequence<std::pair<std::string, std::string>>::value), "");
    static_assert((std::is_same<typename x3::traits::attribute_category<std::pair<std::string, std::string>>::type
                   , x3::traits::tuple_attribute>::value), "");
    
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
            std::cout << pair.first << ':' << pair.second << std::endl;
          }

        auto iterator = fusion::at_c<5>(attr).find("UserAuthToken");
        if(iterator != fusion::at_c<5>(attr).end())
          {
            hub_socket.connect(hub_endpoint);
            hub_socket.set_option(boost::asio::ip::tcp::no_delay(true));
            {
              std::string jabber_plain =
                "<stream:stream to='connect.logitech.com'"
                " xmlns:stream='http://etherx.jabber.org/streams'"
                " xmlns='jabber:client' xml:lang='en' version='1.0'>\r\n"
                "<auth xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\" mechanism=\"PLAIN\">\r\n"
                + encode64("guest\0gatorade")
                + "</auth>\r\n"
                ;

            boost::system::error_code ec;
            boost::asio::write(hub_socket, boost::asio::const_buffers_1(&jabber_plain[0], jabber_plain.size()), ec);

            assert(!ec);
            
            size = hub_socket.read_some(boost::asio::mutable_buffers_1(&buffer[0], buffer.size()));

            std::cout << "read ";
            std::copy(buffer.begin(), buffer.begin() + size, std::ostream_iterator<char>(std::cout));
            std::cout << std::endl;
            }
            
            auth_token = iterator->second;
            std::cout << "token " << auth_token << std::endl;

            std::string jabber_id =
              "<iq type=\"get\" id=\"12345678-1234-5678-1234-123456789012-1\">"
              "<oa xmlns=\"connect.logitech.com\" mime=\"vnd.logitech.connect/vnd.logitech.pair\">"
              "token="
              + auth_token
              + ":name=foo#iOS6.0.1#iPhone</oa></iq>"
              ;

            std::cout << jabber_id;


            boost::system::error_code ec;
            boost::asio::write(hub_socket, boost::asio::const_buffers_1(&jabber_id[0], jabber_id.size()), ec);

            std::cout << "message " << ec << std::endl;
            assert(!ec);

            std::array<char, 4096> buffer;
            std::size_t size =
              hub_socket.read_some(boost::asio::mutable_buffers_1(&buffer[0], buffer.size()));
                                  // , [this] (boost::system::error_code const& ec, std::size_t size)
                                  // {
                                  //   std::cout << "async read ("<< offset << ',' << size-offset << ")" << std::endl;;
                                  //   std::copy(buffer.begin() + offset, buffer.begin() + size
                                  //             , std::ostream_iterator<char>(std::cout));
                                  //   std::endl(std::cout);
                                  //   offset = size;
                                  //   return false;
                                  // }
                                  // , [] (boost::system::error_code const&, std::size_t) {});


            std::cout << "read ";
            std::copy(buffer.begin(), buffer.begin() + size, std::ostream_iterator<char>(std::cout));
            std::cout << std::endl;

            size =
              hub_socket.read_some(boost::asio::mutable_buffers_1(&buffer[0], buffer.size()));
                                  // , [this] (boost::system::error_code const& ec, std::size_t size)
                                  // {
                                  //   std::cout << "async read ("<< offset << ',' << size-offset << ")" << std::endl;;
                                  //   std::copy(buffer.begin() + offset, buffer.begin() + size
                                  //             , std::ostream_iterator<char>(std::cout));
                                  //   std::endl(std::cout);
                                  //   offset = size;
                                  //   return false;
                                  // }
                                  // , [] (boost::system::error_code const&, std::size_t) {});


            std::cout << "read ";
            std::copy(buffer.begin(), buffer.begin() + size, std::ostream_iterator<char>(std::cout));
            std::cout << std::endl;
            
            //std::string token = 
            
            hub_socket.close();
            hub_socket.open(boost::asio::ip::tcp::v4());
            hub_socket.connect(hub_endpoint);
            
            std::string jabber_plain =
              "<stream:stream to='connect.logitech.com'"
              " xmlns:stream='http://etherx.jabber.org/streams'"
              " xmlns='jabber:client' xml:lang='en' version='1.0'>\r\n"
              "<auth xmlns=\"urn:ietf:params:xml:ns:xmpp-sasl\" mechanism=\"PLAIN\">\r\n"
              + encode64("2da85ac6-2999-4019-8a60-a620910f64d3""\0""2da85ac6-2999-4019-8a60-a620910f64d3")
              + "</auth>\r\n"
              ;
            
            boost::asio::write(hub_socket, boost::asio::const_buffers_1(&jabber_plain[0], jabber_plain.size()), ec);

            assert(!ec);
            
            size = hub_socket.read_some(boost::asio::mutable_buffers_1(&buffer[0], buffer.size()));

            std::cout << "read ";
            std::copy(buffer.begin(), buffer.begin() + size, std::ostream_iterator<char>(std::cout));
            std::cout << std::endl;

            std::cout << "send command" << std::endl;

            std::string command =
              "<iq type=\"get\" id=\""
              "12345678-1234-5678-1234-123456789012-1"
              "\"><oa xmlns=\"connect.logitech.com\" mime=\"vnd.logitech.harmony/vnd.logitech.harmony.engine?"
              "holdAction\">action={\"type\"::\"IRCommand\",\"deviceId\"::\""
              + device +
              "\",\"command\"::\""
              "PowerToggle"
              "\"}:status=press</oa></iq>"
              ;

            std::cout << "sending command" << std::endl;
            
            boost::asio::write(hub_socket, boost::asio::const_buffers_1(&command[0], command.size()), ec);

            assert(!ec);
            
            size = hub_socket.read_some(boost::asio::mutable_buffers_1(&buffer[0], buffer.size()));

            std::cout << "read ";
            std::copy(buffer.begin(), buffer.begin() + size, std::ostream_iterator<char>(std::cout));
            std::cout << std::endl;
            //   + encode64()
            //   ;
          }
      }
      else
        std::cout << "error?" << std::endl;
    }
    else
      std::cout << "no parse" << std::endl;
  }


  std::array<char, 4096*1024> buffer;
  std::size_t offset;
  boost::shared_ptr<struct ssl_state> ssl_state_;
  boost::asio::ip::tcp::socket hub_socket;
  std::string hub_hostname, device, email, password;
  std::function<void(std::string, std::vector<argument_variant>)> callback;
  boost::asio::ip::tcp::endpoint auth_endpoint, hub_endpoint;
  std::string auth_token;
};
    
} }

#endif
