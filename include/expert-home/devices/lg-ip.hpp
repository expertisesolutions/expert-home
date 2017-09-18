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

namespace lg_ip_detail {

struct task
{
  boost::asio::ip::tcp::socket socket;
  std::function<void(std::string, std::vector<argument_variant>)>* callback;
  std::array<char, 4096> buffer;
  

  task(boost::asio::io_service& service, std::function<void(std::string, std::vector<argument_variant>)>* callback)
    : socket(service), callback(callback) {}
};

void connection_read(boost::shared_ptr<struct task> task);
  
void connection_data(boost::shared_ptr<struct task> task, boost::system::error_code ec, std::size_t size)
{
  if(size)
    {
      std::cout << "LG watch Read " << size << std::endl;

      namespace x3 = boost::spirit::x3;
      namespace fusion = boost::fusion;

      fusion::vector5<std::string, std::string, char, char, std::vector<fusion::vector2<std::string, std::string>>>
                     attr;
      auto iterator = task->buffer.begin()
        , last = task->buffer.begin() + size;
      if(x3::parse(iterator, last
                   , http_parsers::http::request_line
                   >> *http_parsers::http::header
                   >> x3::omit["\r\n"]
                   , attr))
      {
        std::cout << "Received message "
                  << fusion::at_c<0>(attr)
                  << ' '
                  << fusion::at_c<1>(attr)
                  << std::endl;
        if(fusion::at_c<0>(attr) == "POST"
           && fusion::at_c<1>(attr) == "/hdcp/api/event")
        {
          std::cout << "Event from LG TV. Content: " << std::endl;
          std::copy(iterator, last
                    , std::ostream_iterator<char>(std::cout));
          std::cout << std::endl;

          std::string type;
          fusion::vector2<std::string, std::string&> attr(std::string{}, type);
          if(x3::parse(iterator, last
                       , x3::omit[x3::lit("<?xml version=\"1.0\" encoding=\"utf-8\"?><event><session>")]
                       >> +x3::digit
                       >> x3::omit[x3::lit("</session><name>")]
                       >> +(x3::char_ - '<')
                       >> x3::omit["</name>"]
                       , attr))
          {
            std::cout << "Event " << type << std::endl;
            if(type == "ChannelChanged")
            {
              std::cout << "parse rest of message" << std::endl;
              (*task->callback)(type, {});
            }
            else if(type == "byebye" || type == "CursorVisible")
            {
              (*task->callback)(type, {});
            }
            else
            {
              std::cout << "Unknown event! ================================================" << std::endl;
            }
          }
        }
        else
        {
          std::cout << "Different URL " << fusion::at_c<1>(attr) << std::endl;
        }
      }
      else
        std::cout << "Failed parsing message" << std::endl;
      
      connection_read(task);
    }
  if(ec)
    {
      std::cout << "LG watch errro " << ec.message() << std::endl;
    }
}

void connection_read(boost::shared_ptr<struct task> task)
{
  boost::asio::async_read(task->socket, boost::asio::mutable_buffers_1(&task->buffer[0], task->buffer.size())
                          , std::bind(&connection_data, task, std::placeholders::_1
                                      , std::placeholders::_2));
}
  
void accept_connections(boost::asio::ip::tcp::acceptor* acceptor
                        , std::function<void(std::string, std::vector<argument_variant>)>* callback)
{
  boost::shared_ptr<struct task> task(new struct task(acceptor->get_io_service(), callback));
  acceptor->async_accept
    (task->socket
     , [=] (boost::system::error_code ec)
     {
       std::cout << "accepted connection LG watch" << std::endl;
       if(!ec)
       {
         connection_read(task);
         accept_connections(acceptor, callback);
       }
       else
         std::cout << "Error watching " << ec.message() << std::endl;
     });
}
  
}  

  /* NETFLIX

  POST /hdcp/api/dtv_wifirc HTTP/1.1
Host: 192.168.33.54:8080
Content-Type: application/atom+xml
Content-Length: 144
Connection: Keep-Alive

<?xml version="1.0" encoding="utf-8"?><event><session>502017796</session><type>CursorVisible</type><value>false</value><mode>auto</mode></event>
<?xml version="1.0" encoding="utf-8"?><event><session>1535748147</session><type>CursorVisible</type><value>true</value><mode>auto</mode></event>

<?xml version="1.0" encoding="utf-8"?><command><session>502017796</session><type>HandleTouchClick</type></command>

<?xml version="1.0" encoding="utf-8"?><command><session>502017796</session><type>HandleTouchMove</type><x>1000</x><y>300</y></command>
<?xml version="1.0" encoding="utf-8"?><command><session>454442430</session><type>HandleTouchMove</type><x>-1000</x><y>-1000</y></command>

<?xml version="1.0" encoding="utf-8"?><command><session>502017796</session><type>HandleTouchMove</type><x>-1000</x><y>-1000</y></command>

  */

  
  // NETflix <?xml version="1.0" encoding="utf-8"?><event><session>502017796</session><name>ChannelChanged</name><chname>䜎潬潢䠠D</chname><physicalNum>29</physicalNum><sourceIndex>1</sourceIndex><minor>1</minor><major>4</major><chtype>terrestrial</chtype></event>
  // NETFLIX Open <?xml version="1.0" encoding="utf-8"?><event><session>502017796</session><name>ChannelChanged</name><chname>䜎潬潢䠠D</chname><physicalNum>29</physicalNum><sourceIndex>1</sourceIndex><minor>1</minor><major>4</major><chtype>terrestrial</chtype></event>
  
  // command channel change
  // "/hdcp/api/dtv_wifirc","<?xml version=\"1.0\" encoding=\"utf-8\"?><command><session>#{$lgtv[:session]}</session><type>HandleChannelChange</type><major>#{assigned_no}</major><minor>#{real_no}</minor><sourceIndex>1</sourceIndex><physicalNum>#{uhf_no}</physicalNum></command>",$headers)

  // http://developer.lgappstv.com/TV_HELP/index.jsp?topic=%2Flge.tvsdk.references.book%2Fhtml%2FUDAP%2FUDAP%2FHandleChannelChange.htm

  // HDMI1 <?xml version="1.0" encoding="utf-8"?><event><session>502017796</session><name>ChannelChanged</name><chname></chname><physicalNum>255</physicalNum><sourceIndex>8</sourceIndex><minor>65520</minor><major>65520</major><chtype></chtype></event>

    
struct lg_ip
{
  lg_ip(boost::asio::io_service& service, std::string hostname, std::string password)
    : outstanding_operation(false), socket(service)
    , acceptor(service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 8080))
    , hostname(hostname), password(password)
  {
    boost::asio::ip::tcp::resolver resolver(socket.get_io_service());
    {
      boost::asio::ip::tcp::resolver::iterator
        iterator = resolver.resolve(boost::asio::ip::tcp::resolver::query{hostname, "8080"});
      if(iterator != boost::asio::ip::tcp::resolver::iterator())
        endpoint = *iterator;
      else
        throw std::runtime_error("");
    }
  }

  struct command
  {
    std::string command;
    std::vector<argument_variant> args;
  };
  
  void command_connect_handler(boost::system::error_code ec)
  {
    assert(!command_queue.empty());
    assert(outstanding_operation);
    std::cout << "command_connect_handler" << std::endl;
    if(!ec)
    {
      boost::system::error_code ec;
      command_run(command_queue.front(), ec);

      if(ec)
      {
        catastrophe();
      }
      else // this should happen only after checking return
      {
        std::cout << "Command executed OK" << std::endl;
        // command_queue.erase(command_queue.begin());
        // finish_operation();
        // outstanding_operation = false;
        // socket.close();
        return;
      }
    }
    socket.async_connect(endpoint, std::bind(&lg_ip::command_connect_handler, this, std::placeholders::_1));
  }

  //$sess.get("/hdcp/api/data?target=cur_channel&session=#{$lgtv[:session]}",$headers)
  
  void command_run(command const& c, boost::system::error_code& ec)
  {
    std::function<void(std::size_t)> command_response;
    
    std::cout << "connection OK command: " << c.command << std::endl;
    std::string request;
    if(c.command == "HandleKeyInput" && c.args.size() == 1)
    {
      std::string body;
      namespace x3 = boost::spirit::x3;
      namespace fusion = boost::fusion;
      if(x3::generate(std::back_insert_iterator<std::string>(body)
                      , x3::omit[x3::lit("<?xml version=\"1.0\" encoding=\"utf-8\"?><command><session>")]
                      >> x3::int_
                      >> x3::omit[x3::lit("</session><type>")]
                      >> +x3::char_
                      >> x3::omit[x3::lit("</type><value>")]
                      >> (+x3::char_ | x3::int_)
                      >> x3::omit[x3::lit("</value></command>")]
                      , fusion::vector3<int, std::string const&, /*std::string*/eh::argument_variant>
                      (session, c.command, /*boost::get<std::string>(*/c.args[0]/*)*/)))
      {
        std::cout << "Generated body " << std::endl;
        std::copy(body.begin(), body.end(), std::ostream_iterator<char>(std::cout));
        std::cout << std::endl;

        std::stringstream message;
        message <<
          "POST /hdcp/api/dtv_wifirc HTTP/1.1\r\n"
          "Host: 192.168.33.54:8080\r\n"
          "Content-Type: application/atom+xml\r\n"
          "Content-Length: " << body.size() <<
          "\r\nConnection: Close\r\n\r\n" <<
          body
          ;
        request = message.str();

        command_response = std::bind(&lg_ip::command_handle_key_response, this, std::placeholders::_1);
      }
      else
        throw std::runtime_error("Failed generation");
    }
    else if(c.command == "HandleTouchClick")
    {
      std::stringstream body_stream;
      body_stream <<
        "<?xml version=\"1.0\" encoding=\"utf-8\"?><command><session>"
              << session <<
        "</session><type>HandleTouchClick</type></command>";
      std::string body = body_stream.str();
      std::stringstream message;
      message <<
        "POST /hdcp/api/dtv_wifirc HTTP/1.1\r\n"
        "Host: 192.168.33.54:8080\r\n"
        "Content-Type: application/atom+xml\r\n"
        "Content-Length: " << body.size() << "\r\n"
        "Connection: Close\r\n\r\n"
              << body
        ;
      request = message.str();

      std::cout << "Generated request " << std::endl;
      std::copy(request.begin(), request.end(), std::ostream_iterator<char>(std::cout));
      std::cout << std::endl;

      command_response = std::bind(&lg_ip::command_handle_key_response, this, std::placeholders::_1);
    }
    else if(c.command == "CurrentChannel")
    {
      std::stringstream message;
      message <<
        "GET /hdcp/api/data?target=cur_channel&session="
              << session <<
        " HTTP/1.1\r\n"
        "Host: 192.168.33.54:8080\r\n"
        "Content-Type: application/atom+xml\r\n"
        "Content-Length: 0\r\n"
        "Connection: Close\r\n\r\n"
        ;
      request = message.str();

      std::cout << "Generated request " << std::endl;
      std::copy(request.begin(), request.end(), std::ostream_iterator<char>(std::cout));
      std::cout << std::endl;

      command_response = std::bind(&lg_ip::command_current_channel_response, this, std::placeholders::_1);
    }
    /*else if(c.command == "CursorVisible" && c.args.size() == 1 && boost::get<int>(&c.args[0]))
    {
      std::string body;
      namespace x3 = boost::spirit::x3;
      namespace fusion = boost::fusion;
      if(x3::generate(std::back_insert_iterator<std::string>(body)
                      , x3::omit[x3::lit("<?xml version=\"1.0\" encoding=\"utf-8\"?><event><session>")]
                      >> x3::int_
                      >> x3::omit[x3::lit("</session><name>")]
                      >> +x3::char_
                      >> x3::omit[x3::lit("</name><value>")]
                      >> +x3::char_
                      >> x3::omit[x3::lit("</value><mode>auto</mode></event>")]
                      , fusion::vector3<int, std::string const&, const char*>
                      (session, c.command, boost::get<int>(c.args[0]) ? "true" : "false")))
      {
        std::stringstream message;
        message <<
          "POST /hdcp/api/dtv_wifirc HTTP/1.1\r\n"
          "Host: 192.168.33.54:8080\r\n"
          "Content-Type: application/atom+xml\r\n"
          "Content-Length: " << body.size() <<
          "\r\nConnection: Close\r\n\r\n" <<
          body
          ;
        request = message.str();

        std::cout << "request" << std::endl;
        std::copy(request.begin(), request.end(), std::ostream_iterator<char>(std::cout));
        std::cout << std::endl;

        command_response = std::bind(&lg_ip::command_handle_key_response, this, std::placeholders::_1);
      }
      else
        throw std::runtime_error("Failed generation");
        }*/
    else if(c.command == "HandleTouchMove" && c.args.size() == 2)
    {
      std::string body;
      namespace x3 = boost::spirit::x3;
      namespace fusion = boost::fusion;
      if(x3::generate(std::back_insert_iterator<std::string>(body)
                      , x3::omit[x3::lit("<?xml version=\"1.0\" encoding=\"utf-8\"?><command><session>")]
                      >> x3::int_
                      >> x3::omit[x3::lit("</session><type>")]
                      >> +x3::char_
                      >> x3::omit[x3::lit("</type><x>")]
                      >> (x3::int_ | x3::int_)
                      >> x3::omit[x3::lit("</x><y>")]
                      >> (x3::int_ | x3::int_)
                      >> x3::omit[x3::lit("</y></command>")]
                      , fusion::vector4<int, std::string const&, eh::argument_variant, eh::argument_variant>
                      (session, c.command, c.args[0], c.args[1])))
      {
        std::stringstream message;
        message <<
          "POST /hdcp/api/dtv_wifirc HTTP/1.1\r\n"
          "Host: 192.168.33.54:8080\r\n"
          "Content-Type: application/atom+xml\r\n"
          "Content-Length: " << body.size() <<
          "\r\nConnection: Close\r\n\r\n" <<
          body
          ;
        request = message.str();

        std::cout << "request" << std::endl;
        std::copy(request.begin(), request.end(), std::ostream_iterator<char>(std::cout));
        std::cout << std::endl;

        command_response = std::bind(&lg_ip::command_handle_key_response, this, std::placeholders::_1);
      }
      else
        throw std::runtime_error("Failed generation");
    }
    else
      throw std::runtime_error("Unknown command");

    boost::asio::write(socket, boost::asio::const_buffers_1(&request[0], request.size()), ec);

    if(!ec)
    {
      std::cout << "Command " << c.command << " sent succesfully" << std::endl;

      boost::asio::async_read(socket, boost::asio::mutable_buffers_1(buffer.begin(), buffer.size())
                              // , [this] (boost::system::error_code const& ec, std::size_t size)
                              // {
                              //   auto last = buffer.begin() + size;
                              //   return ec || size == 1024 || std::find(buffer.begin(), last, '\r') != last;
                              // }
                              // , boost::bind(&denon_ip::handler, this, _1, _2)
                              , [this, command_response] (boost::system::error_code const& ec, std::size_t size)
                              {
                                // auto last = buffer.begin() + size;
                                // return ec || size == 1024 || std::find(buffer.begin(), last, '\r') != last;
                                if(size)
                                {
                                  std::cout << "received " << size << std::endl;
                                  command_response(size);
                                }
                                else if(ec)
                                {
                                  catastrophe();
                                }
                              }
                              );
    }
  }

  void send_command(std::string const& c, std::vector<argument_variant> const& args)
  {
    std::cout << "queue command " << c << std::endl;
    command_queue.push_back(command{c, args});

    if(!outstanding_operation)
    {
      assert(!socket.is_open());
      std::cout << "Socket not open!" << std::endl;
      outstanding_operation = true;
      socket.open(boost::asio::ip::tcp::v4());
      socket.async_connect(endpoint, std::bind(&lg_ip::command_connect_handler, this, std::placeholders::_1));
    }
  }

  void connect_handler(boost::system::error_code ec)
  {
    std::cout << "connect_handler" << std::endl;
    if(!ec)
    {
      std::cout << "connect_handler OK" << std::endl;
      std::string body = "<?xml version=\"1.0\" encoding=\"utf-8\"?><auth><type>AuthReq</type><value>"
        + password + "</value></auth>";
      std::stringstream message;
      message <<
        "POST /hdcp/api/auth HTTP/1.1\r\n"
        "Content-Type: application/atom+xml\r\n"
        "Content-Length: " << body.size() << "\r\n" <<
        "Host: 192.168.33.54:8080\r\n" <<
        "Connection: Keep-Alive\r\n\r\n" << body;

      std::string request = message.str();
      boost::asio::write(socket, boost::asio::const_buffers_1(&request[0], request.size())
                         // , [this] (boost::system::error_code const& ec, std::size_t size)
                         // {
                         //   std::cout << "sent I think" << std::endl;
                         // }
                         , ec
                         );

      if(!ec)
      {
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
                                    this->auth_response_handle(size);
                                  }
                                  else if(ec)
                                  {
                                    std::cout << "error size " << size << " error " << ec.message() << std::endl;
                                  }
                                }
                                );
      }
      else
        catastrophe();
    }
    else
      socket.async_connect(endpoint, std::bind(&lg_ip::connect_handler, this, std::placeholders::_1));
  }
  
  void watch(std::function<void(std::string, std::vector<argument_variant>)> function)
  {
    outstanding_operation = true;
    socket.async_connect(endpoint, std::bind(&lg_ip::connect_handler, this, std::placeholders::_1));

    lg_ip_detail::accept_connections(&acceptor, &callback);
    
    this->callback = std::move(function);
  }

  void auth_response_handle(std::size_t size)
  {
    std::cout << "Handle LG" << std::endl;
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

      callback("poweron", {});
      
      finish_operation();
    }
    else
    {
      std::cout << "error parsing" << std::endl;
      std::copy(buffer.begin(), buffer.begin() + size
                , std::ostream_iterator<char>(std::cout));
      std::endl(std::cout);
    }
    // socket.close();
  }

  void catastrophe()
  {
    std::cout << "Catastrophic error, restarting session and trying all over again" << std::endl;

    socket.close();
    outstanding_operation = true;
    socket.async_connect(endpoint, std::bind(&lg_ip::connect_handler, this, std::placeholders::_1));
  }

  void command_handle_key_response(std::size_t size)
  {
    assert(!!outstanding_operation);
    // std::cout << "command response size " << size << std::endl;
    // std::copy(buffer.begin(), buffer.begin() + size, std::ostream_iterator<char>(std::cout));
    // std::endl(std::cout);
    namespace x3 = boost::spirit::x3;
    namespace fusion = boost::fusion;
    auto iterator = buffer.begin();
    boost::fusion::vector3<int, std::string, unsigned int> attr;
    // <?xml version="1.0" encoding="utf-8"?><envelope><HDCPError>200</HDCPError><HDCPErrorDetail>OK</HDCPErrorDetail><session>114859659</session></envelope>
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
                 , attr)
       && fusion::at_c<0>(attr) >= 200 && fusion::at_c<0>(attr) < 300)
    {
      command_queue.erase(command_queue.begin());
      std::cout << "response was OK from LG" << std::endl;
      finish_operation();
    }
    else
    {
      std::cout << "can't parse response" << std::endl;
      std::copy(buffer.begin(), buffer.begin() + size
                , std::ostream_iterator<char>(std::cout));
      std::endl(std::cout);
      catastrophe();
    }
  }

  void command_current_channel_response(std::size_t size)
  {
    assert(!!outstanding_operation);
    // std::cout << "command response size " << size << std::endl;
    // std::copy(buffer.begin(), buffer.begin() + size, std::ostream_iterator<char>(std::cout));
    // std::endl(std::cout);
    namespace x3 = boost::spirit::x3;
    namespace fusion = boost::fusion;
    auto iterator = buffer.begin();
    boost::fusion::vector8<int, std::string, unsigned int
                           , std::string
                           , int, int
                           , int, int> attr;
    // <?xml version="1.0" encoding="utf-8"?><envelope><HDCPError>200</HDCPError><HDCPErrorDetail>OK</HDCPErrorDetail><session>454442430</session><data><type>terrestrial</type><major>4</major><minor>1</minor><sourceIndex>1</sourceIndex><physicalNum>29</physicalNum
    if(x3::parse(iterator, buffer.begin() + size
                 , x3::omit[+(x3::char_ - ("\r\n\r\n"))]
                 >> "\r\n\r\n"
                 >> "<?xml version=\"1.0\" encoding=\"utf-8\"?><envelope><HDCPError>"
                 >> x3::int_
                 >> "</HDCPError><HDCPErrorDetail>"
                 >> (+(x3::char_ - '<'))
                 >> "</HDCPErrorDetail><session>"
                 >> x3::uint_
                 >> "</session>"
                 >> "<data><type>"
                 >> (+(x3::char_ - '<'))
                 >> "</type><major>"
                 >> x3::uint_
                 >> "</major><minor>"
                 >> x3::uint_
                 >> "</minor><sourceIndex>"
                 >> x3::uint_
                 >> "</sourceIndex><physicalNum>"
                 >> x3::uint_
                 >> "</physicalNum><name>"
                 >> x3::omit[(*(x3::char_ - '<'))]
                 >> "</name></data></envelope>"
                 , attr)
       && fusion::at_c<0>(attr) >= 200 && fusion::at_c<0>(attr) < 300)
    {
      command_queue.erase(command_queue.begin());
      std::cout << "response was OK from LG" << std::endl;
      finish_operation();

      auto type = [&] (auto v) { return fusion::at_c<3>(v); };
      auto (major) = [&] (auto v) { return fusion::at_c<4>(v); };
      auto (minor) = [&] (auto v) { return fusion::at_c<5>(v); };
      auto source_index = [&] (auto v) { return fusion::at_c<6>(v); };
      auto physical_number = [&] (auto v) { return fusion::at_c<7>(v); };
      
      std::cout << "Channel Type " << type(attr) << std::endl;
      std::cout << "Channel Major " << (major)(attr) << std::endl;
      std::cout << "Channel Minor " << (minor)(attr) << std::endl;
      std::cout << "Channel source_index " << source_index(attr) << std::endl;
      std::cout << "Channel Physical number " << physical_number(attr) << std::endl;

      callback("currentchannel", {type(attr), (major)(attr), (minor)(attr), source_index(attr), physical_number(attr)});
    }
    else
    {
      std::cout << "can't parse response" << std::endl;
      std::copy(buffer.begin(), buffer.begin() + size
                , std::ostream_iterator<char>(std::cout));
      std::endl(std::cout);
      catastrophe();
    }
  }
  
  void finish_operation()
  {
    std::cout << "Operation finished" << std::endl;
    socket.close();
    if(!command_queue.empty())
    {
      socket.open(boost::asio::ip::tcp::v4());
      socket.async_connect(endpoint, std::bind(&lg_ip::command_connect_handler, this, std::placeholders::_1));
    }
    else
      outstanding_operation = false;
  }
  
  bool outstanding_operation;
  std::vector<command> command_queue;
  std::array<char, 4096> buffer;
  boost::asio::ip::tcp::socket socket;
  boost::asio::ip::tcp::acceptor acceptor;
  std::string hostname;
  std::string password;
  std::vector<std::string> commands;
  boost::asio::ip::tcp::endpoint endpoint;
  unsigned int session;
  std::function<void(std::string, std::vector<argument_variant>)> callback;
};

} }

#endif

