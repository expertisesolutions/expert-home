// Copyright Felipe Magno de Almeida 2016-2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef CALAOS_COMMUNICATION_API_HPP
#define CALAOS_COMMUNICATION_API_HPP

#include <beast/http.hpp>
#include <beast/websocket.hpp>
//#include <beast/core/to_string.hpp>
#include <functional>

#include <experimental/optional>
#include <beast/websocket/ssl.hpp>

#include <utility>

namespace eh { namespace calaos {

struct http_communication
{
  struct client;
  
  using endpoint_type = boost::asio::ip::tcp::endpoint;
  using socket_type  = boost::asio::ip::tcp::socket;
  using ssl_socket_type  = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
  using error_code = boost::system::error_code;


  struct socket_wrapper
  {
    socket_wrapper(socket_type&& socket)
      : socket(std::move(socket))
    {
    }

    socket_type& operator*() { return socket; }
    socket_type* operator->() { return &socket; }
    socket_type const& operator*() const { return socket; }
    socket_type const* operator->() const { return &socket; }

    socket_type socket;
  };
  
  typedef boost::variant<socket_wrapper, std::unique_ptr<ssl_socket_type> > variant_socket_type;
  struct ssl_context_type : boost::asio::ssl::context
  {
    ssl_context_type()
      : context(boost::asio::ssl::context::tlsv12_server)
    {
      set_options(boost::asio::ssl::context::default_workarounds);
      set_verify_mode(verify_none);
      use_certificate_file("newcert.pem", pem);
      use_private_key_file("privkey.pem", pem);
      use_tmp_dh_file("dh512.pem");
    }
  } ssl_context;
  boost::asio::ip::tcp::acceptor acceptor;
  boost::asio::ip::tcp::acceptor ssl_acceptor;
  socket_type socket;
  std::unique_ptr<ssl_socket_type> ssl_socket;
  boost::asio::io_service* io_service;
  std::function<void(std::string const&, client&)> function;
  std::function<void(std::map<std::string, std::string> const&, client&)> function_url;

  using req_type = beast::http::request<beast::http::string_body>;
  using resp_type = beast::http::response<beast::http::string_body>;
  // using websocket_stream_type = boost::variant<boost::blank, beast::websocket::stream<socket_type&>
  //                                              , beast::websocket::stream<ssl_socket_type&>>;
  using websocket_stream_type = beast::websocket::stream<socket_type>;

  struct client
  {
    client(client&&) = default;
    
    variant_socket_type socket;
    std::function<void(std::string const&, client&)> function;
    std::function<void(std::map<std::string, std::string> const&, client&)> function_url;
    beast::multi_buffer streambuf;
    struct websocket_data
    {
      websocket_stream_type stream;
      //beast::websocket::opcode op;

      friend void swap(websocket_data& lhs, websocket_data& rhs)
      {
        using std::swap;
        swap(lhs.stream, rhs.stream);
        // swap(lhs.op, rhs.op);
      }
    };
    std::experimental::optional<websocket_data> websocket;
    req_type req;

    struct async_read_visitor
    {
      typedef void result_type;
      template <typename S>
      void operator()(S& socket, beast::multi_buffer& streambuf, req_type& req
                      , client& self) const
      {
        async_read(*socket, streambuf, req
                   , std::bind(&client::on_read, &self
                               , std::placeholders::_1));
      }  
    };

    struct close_visitor
    {
      typedef void result_type;
      void operator()(socket_wrapper& socket) const
      {
        (*socket).close();
      }
      void operator()(std::unique_ptr<ssl_socket_type>& socket) const
      {
        socket->lowest_layer().close();
      }
    };

    void close()
    {
      close_visitor close_v;
      socket.apply_visitor(close_v);
    }
    
    void read()
    {
      std::cout << "reading" << std::endl;
      auto handler = std::bind(async_read_visitor{}, std::placeholders::_1
                               , std::ref(streambuf), std::ref(req), std::ref(*this));
      socket.apply_visitor(handler);
    }

    struct async_accept_visitor
    {
      typedef void result_type;
      template <typename S>
      void operator()(beast::websocket::stream<S>& stream, req_type& req, client& self) const
      {
        stream.async_accept(req, std::bind(&client::on_upgrade, &self, std::placeholders::_1));
      }
      template <typename...Args>
      void operator()(boost::blank const&, Args&&...) const
      {
        throw std::runtime_error("variant is blank");
      }
    };

    struct initialize_websocket_visitor
    {
      typedef websocket_data result_type;
      template <typename S>
      static beast::websocket::stream<S> make_websocket(S& socket)
      { return beast::websocket::stream<S>{socket}; }
      websocket_data operator()(socket_wrapper& socket) const
      {
        return websocket_data{/*make_websocket(*/websocket_stream_type{std::move(*socket)}/*)*/ /*, {}*/};
      }
      websocket_data operator()(std::unique_ptr<ssl_socket_type>& socket) const
      {
        throw -1;
      }
    };
    
    void on_read(error_code const& ec)
    {
      if(!ec)
        {
          std::cout << "read" << std::endl;
          std::cout << "request " << req << std::endl;
          if(beast::websocket::is_upgrade(req))
            {
              std::cout << "is upgrade" << std::endl;
              // initialize_websocket_visitor init_handler;
              // std::experimental::optional<websocket_data> new_data{socket.apply_visitor(init_handler)};
              // websocket.swap(new_data);
              // //websocket->stream.async_accept(req, std::bind(&client::on_upgrade, this, std::placeholders::_1));
              auto handler = std::bind(async_accept_visitor(), std::placeholders::_1
                                       , std::ref(req), std::ref(*this));
              // // websocket->stream.apply_visitor(handler);
              // handler(websocket->stream);

              assert(!!boost::get<socket_wrapper>(&socket));
              
              websocket = websocket_data
                {websocket_stream_type
                 {std::move(**boost::get<socket_wrapper>(&socket))}/*, {}*/};
              handler(websocket->stream);
            }
          else
            {
              std::cout << "is a request with url " << req.target() << std::endl;
              auto question_mark = std::find(req.target().begin(), req.target().end(), '?');
              std::string api(req.target().begin(), question_mark);
              if(req.method() == beast::http::verb::get && api == "/api" && question_mark != req.target().end())
              {
                std::cout << "is api" << std::endl;
                
                std::map<std::string, std::string> map;
                  
                auto iterator = std::next(question_mark);
                while(iterator != req.target().end())
                {
                  auto separator = std::find(iterator, req.target().end(), '=');
                  if(separator != req.target().end())
                  {
                    std::string key(iterator, separator);
                    iterator = ++separator;
                    separator = std::find(iterator, req.target().end(), '&');
                    std::string value(iterator, separator);
                    map.insert(std::make_pair(key, value));
                    if(separator != req.target().end())
                      iterator = ++separator;
                  }
                  else
                    break;
                }

                for(auto&& pair : map)
                {
                  std::cout << "key " << pair.first << " value " << pair.second << std::endl;
                }
                  
                function_url(map, *this);
              }
              else if(req.method() == beast::http::verb::post && (api == "/api" || api == "/api.php" || api == "/") && question_mark == req.target().end())
              {
                std::cout << "A post with " << req.body << std::endl;
                function(req.body, *this);
                req = {};
              }

              read();
            }
        }
      else
        {
          std::cout << "not read " << ec.message() << std::endl;
        }
    }

    struct websocket_read_visitor
    {
      typedef void result_type;
      template <typename S>
      void operator()(beast::websocket::stream<S>& stream/*, beast::websocket::opcode& op*/
                      , beast::multi_buffer& streambuf, client& self) const
      {
        stream.async_read(/*op, */streambuf
                          , std::bind(&client::on_websocket_read, &self, std::placeholders::_1));
      }
      template <typename...Args>
      void operator()(boost::blank const&, Args&&...) const
      {
        throw std::runtime_error("variant is blank");
      }
    };
    
    void websocket_read()
    {
      auto handle = std::bind(websocket_read_visitor(), std::placeholders::_1
                              /*, std::ref(websocket->op)*/, std::ref(streambuf)
                              , std::ref(*this));
      // websocket->stream.apply_visitor(handle);
      handle(websocket->stream);
    }

    void on_upgrade(error_code const& ec)
    {
      if(!ec)
        {
          streambuf.consume(streambuf.size());
          std::cout << "upgraded" << std::endl;
          websocket_read();
        }
      else
        std::cout << "error upgrading " << ec.message()  << std::endl;
    }

    void on_websocket_read(error_code const& ec)
    {
      std::cout << "on_websocket_read" << std::endl;
      if(!ec)
        {
          std::cout << "on_websocket_read " << streambuf.size() << " bytes" << std::endl;
          // std::string data = beast::to_string(streambuf.data());
          std::string data;
          for(auto&& i : streambuf.data())
            data.insert(data.end(), boost::asio::buffer_cast<const char*>(i)
                        , boost::asio::buffer_cast<const char*>(i) + boost::asio::buffer_size(i));
          std::cout << data << std::endl;;
          streambuf.consume(streambuf.size());

          function(data, *this);

          websocket_read();
        }
      else
        std::cout << "fail websocket read error " << ec.message() << std::endl;
    }

    struct websocket_write_visitor
    {
      typedef void result_type;
      template <typename S>
      void operator()(beast::websocket::stream<S>& stream, std::string string) const
      {
        // stream.set_option(beast::websocket::opcode::text);
        stream.write(boost::asio::const_buffers_1(string.data(), string.size()));
      }
      template <typename...Args>
      void operator()(boost::blank const&, Args&&...) const
      {
        throw std::runtime_error("variant is blank");
      }
    };

    void write(std::string const& string)
    {
      if(websocket)
        {
          std::cout << "sending " << string << std::endl;
          auto handle = std::bind(websocket_write_visitor(), std::placeholders::_1, string);
          //websocket->stream.apply_visitor(handle);
          handle(websocket->stream);
        }
    }
  };
  
  std::list<client> clients;

  template <typename F, typename FUrl>
  http_communication(endpoint_type endpoint
                     , endpoint_type ssl_endpoint
                     , F f
                     , FUrl function_url
                     , boost::asio::io_service& io_service)
    : acceptor(io_service)
    , ssl_acceptor(io_service)
    , socket(io_service)
    , ssl_socket(new ssl_socket_type(io_service, ssl_context))
    , io_service(&io_service)
    , function(f)
    , function_url(function_url)
  {
    boost::asio::socket_base::reuse_address option(true);
    acceptor.open(endpoint.protocol());
    acceptor.set_option(option);
    acceptor.bind(endpoint);
    acceptor.listen(socket_type::max_connections);

    ssl_acceptor.open(ssl_endpoint.protocol());
    ssl_acceptor.set_option(option);
    ssl_acceptor.bind(ssl_endpoint);
    ssl_acceptor.listen(socket_type::max_connections);
  }

  void accept()
  {
    std::cout << "accepting" << std::endl;
    normal_accept();
    ssl_accept();
  }

  void normal_accept()
  {
    std::cout << "accepting" << std::endl;
    acceptor.async_accept(socket, std::bind(&http_communication::on_accept, this
                                            , std::placeholders::_1));
  }

  void ssl_accept()
  {
    std::cout << "accepting" << std::endl;
    ssl_acceptor.async_accept(ssl_socket->lowest_layer(), std::bind(&http_communication::on_ssl_accept, this
                                                                    , std::placeholders::_1));
  }
  
  void on_accept(error_code const& ec)
  {
    if(!ec)
      {
        std::cout << "accepted" << std::endl;
        clients.push_back(client{std::move(socket), function, function_url, {}, {}, {}});
        clients.back().read();
        normal_accept();
      }
    else
      {
        std::cout << "not accepted " << ec.message() << std::endl;
      }
  }
#ifdef BOOST_NO_CXX11_RVALUE_REFERENCES
#error BOOST_NO_CXX11_RVALUE_REFERENCES
#endif

  void on_ssl_accept(error_code const& ec)
  {
    if(!ec)
      {
        std::cout << "accepted" << std::endl;
        // error_code ec;
        // ssl_socket->handshake(ssl_socket_type::server, ec);
        // if(ec)
        //   {
        //     std::cout << "Error handshake ssl " << ec.message() << std::endl;
        //   }
        // // else
        //   {
        //     clients.push_back(client{std::move(ssl_socket), function, function_url, {}, {}, {}});
        //     clients.back().read();
        //   }
        // ssl_socket.reset(new ssl_socket_type(*io_service, ssl_context));
        // ssl_accept();
      }
    else
      {
        std::cout << "not accepted " << ec.message() << std::endl;
      }
  }
};
  
} }

#endif
