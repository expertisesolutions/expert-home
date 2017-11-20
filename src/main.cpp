// Copyright Felipe Magno de Almeida 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>

#include <boost/asio.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/trim_all.hpp>
#include <expert-home/server/ssdp.hpp>
#include <expert-home/devices/harmony-device.hpp>
#include <expert-home/devices/output/milight.hpp>
#include <expert-home/devices/rs232.hpp>
#include <expert-home/devices/denon-ip.hpp>
#include <expert-home/devices/lg-ip.hpp>
#include <expert-home/devices/lg-rs232.hpp>
#include <expert-home/devices/input/roku-ip.hpp>
#include <expert-home/devices/lua/device.hpp>
#include <expert-home/devices/lua/harmony-device.hpp>
#include <expert-home/devices/lua/input.hpp>
#include <expert-home/devices/cameras/dahua_ip.hpp>
#include <expert-home/schedule.hpp>
#include <expert-home/calaos/http_communication.hpp>
#include <expert-home/calaos/get_home.hpp>
#include <expert-home/devices/device.hpp>

#include <lua.hpp>

#include <luabind/luabind.hpp>
#include <luabind/tag_function.hpp>

#include <json.hpp>

#include <fstream>

struct print_visit
{
  void operator()(std::string const& string) const
  {
    std::cout << " [string:" << string << "]";
  }

  void operator()(int i) const
  {
    std::cout << " [int:" << i << "]";
  }
};

struct lua_push_visitor
{
  lua_State* L;

  void operator()(std::string const& string) const
  {
    lua_pushstring(L, string.c_str());
  }

  void operator()(int i) const
  {
    lua_pushnumber(L, i);
  }
};

void callback_function(lua_State* L, luabind::object device, std::string command, std::vector<eh::argument_variant> args
                       , luabind::object function)
{
  std::cout << "command " << command << " with " << args.size() << " arguments ";
  for(auto const& arg : args)
  {
    boost::apply_visitor(print_visit(), arg);
  }
  std::endl(std::cout);

  std::cout << "function type " << luabind::type(function) << std::endl;
  if(luabind::type(function) != LUA_TNIL)
  {
    function.push(L);
    device.push(L);
    lua_pushstring(L, command.c_str());
    for(auto const& arg : args)
    {
      boost::apply_visitor(lua_push_visitor{L}, arg);
    }
    if(lua_pcall(L, 2 + args.size(), 0, 0))
    {
      std::cout << "failed calling device handler" << std::endl;

      std::cout << "Error: " << lua_tostring(L, -1) << std::endl;
      lua_pop(L, -1);
    }
  }
  else
  {
    std::cout << "Function is nil" << std::endl;
  }
}

struct overload_interface
{
  virtual void operator()(nlohmann::json json, std::string, eh::calaos::http_communication::client& client) const = 0;
  virtual void operator()(std::map<std::string, std::string>const&, std::string, eh::calaos::http_communication::client&) const = 0;
};

template <typename F>
struct overload_impl : overload_interface
{
  void operator()(nlohmann::json json, std::string msg_id, eh::calaos::http_communication::client& client) const
  {
    f(json, msg_id, client);
  }
  void operator()(std::map<std::string, std::string> const& json, std::string msg_id, eh::calaos::http_communication::client& client) const
  {
    f(json, msg_id, client);
  }
  overload_impl(F&& f) : f(f)
  {
  }

  F f;
};

struct overload
{
  template <typename F>
  overload(F&& f)
    : overload_(new overload_impl<F>(std::forward<F>(f)))
  {
  }

  void operator()(nlohmann::json json, std::string msg_id, eh::calaos::http_communication::client& client) const
  {
    (*overload_)(json, msg_id, client);
  }
  void operator()(std::map<std::string, std::string> const& map, std::string msg_id, eh::calaos::http_communication::client& client) const
  {
    (*overload_)(map, msg_id, client);
  }

  std::unique_ptr<overload_interface> overload_;
};

struct write_resp
{
  typedef void result_type;
  write_resp(eh::calaos::http_communication::resp_type const& resp)
    : resp(resp)
  {}

  template <typename S>
  void operator()(S& socket) const
  {
    std::cout << "just socket" << std::endl;
  }

  void operator()(eh::calaos::http_communication::socket_wrapper& socket) const
  {
    // boost::system::error_code ec;
    beast::http::write(*socket, resp/*, ec*/);
  }

  eh::calaos::http_communication::resp_type const& resp;
};

template <typename L, typename R>
R const& get_string_value(std::pair<L, R> const& pair)
{
  return pair.second;
}
template <typename L, typename R>
R& get_string_value(std::pair<L, R>& pair)
{
  return pair.second;
}
std::string get_string_value(nlohmann::basic_json<> const& json)
{
  return json.get<std::string>();
}

int main()
{
  boost::asio::io_service io_service;

  // boost::asio::ip::udp::socket socket(io_service, boost::asio::ip::udp::v4());
  // // socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));

  // // boost::asio::socket_base::broadcast option(true);
  // // socket.set_option(option);
  // // socket.set_option(boost::asio::ip::multicast::join_group
  // //                   (boost::asio::ip::address::from_string("192.168.0.255")
  // //                    /*, boost::asio::ip::address::from_string("192.168.0.2")*/));
  // socket.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), 20086));

  // // socket.set_option(boost::asio::ip::multicast::join_group
  // //                   (boost::asio::ip::address::from_string("192.168.0.255")
  // //                    /*, boost::asio::ip::address::from_string("192.168.0.2")*/));

  // // boost::asio::ip::udp::endpoint endpoint
  // //   (boost::asio::ip::address::from_string("192.168.1.255"), 20086);
  // // socket.bind(boost::asio::ip::udp::endpoint
  // //             (boost::asio::ip::address::from_string("192.168.1.255"), 20086));
  // // socket.bind(boost::asio::ip::udp::endpoint
  // //             (boost::asio::ip::address::any("192.168.1.255"), 20086));

  // std::array<char, 4096> buffer;

  // std::size_t size =
  //   socket.receive(boost::asio::mutable_buffers_1(&buffer[0], 4096), 0);


  // std::cout << "read ";
  // std::copy(buffer.begin(), buffer.begin() + size, std::ostream_iterator<char>(std::cout));
  // std::cout << std::endl;


  lua_State* L = luaL_newstate();
  luaL_openlibs(L);

  if(luaL_dostring(L, "package.path = package.path .. ';/home/felipe/dev/home-automation/home-automation-old-dell/expert-home/lua/?.lua'\n"
                   "print ('package.path ' .. package.path)"))
    {
      std::cout << "Failed package.path" << std::endl;
      std::cout << "Error: " << lua_tostring(L, -1) << std::endl;
      return -1;
    }
  
  luabind::open(L);
  eh::devices::lua::register_device(L);
  eh::devices::lua::register_harmony_device(L);
  eh::devices::lua::register_input(L);
  eh::register_schedule(io_service, L);

  // std::map<std::string, eh::device::lg_rs232> lg_rs232s;
  // std::map<std::string, eh::device::lg_ip> lgs;
  // std::map<std::string, eh::device::denon_ip> denons;
  std::map<std::string, eh::device::harmony_device> harmony_devices;
  // std::map<std::string, eh::device::roku_ip> roku_ips;
  // std::map<std::string, eh::device::output::milight_device> milights;
  std::map<std::string, eh::device::camera::dahua_ip> cameras;
  std::map<std::string, eh::device::device_any> device_map;

  cameras.insert(std::make_pair("Liz Cama", eh::device::camera::dahua_ip
                                {{boost::asio::ip::address::from_string("192.168.95.2"), 80}, io_service}));
  cameras.insert(std::make_pair("Jantar Exterior", eh::device::camera::dahua_ip
                                {{boost::asio::ip::address::from_string("192.168.95.3"), 80}, io_service}));
  cameras.insert(std::make_pair("Piscina", eh::device::camera::dahua_ip
                                {{boost::asio::ip::address::from_string("192.168.95.4"), 80}, io_service}));
  cameras.insert(std::make_pair("Interfone", eh::device::camera::dahua_ip
                                {{boost::asio::ip::address::from_string("192.168.95.5"), 80}, io_service}));
  cameras.insert(std::make_pair("Carro", eh::device::camera::dahua_ip
                                {{boost::asio::ip::address::from_string("192.168.95.6"), 80}, io_service}));
  cameras.insert(std::make_pair("Liz Microfone", eh::device::camera::dahua_ip
                                {{boost::asio::ip::address::from_string("192.168.95.7"), 80}, io_service}));
  cameras.insert(std::make_pair("Sala Jantar", eh::device::camera::dahua_ip
                                {{boost::asio::ip::address::from_string("192.168.95.8"), 80}, io_service}));
  cameras.insert(std::make_pair("Poste", eh::device::camera::dahua_ip
                                {{boost::asio::ip::address::from_string("192.168.95.9"), 80}, io_service}));
  cameras.insert(std::make_pair("Brinquedoteca", eh::device::camera::dahua_ip
                                {{boost::asio::ip::address::from_string("192.168.95.10"), 80}, io_service}));
  cameras.insert(std::make_pair("Fundos", eh::device::camera::dahua_ip
                                {{boost::asio::ip::address::from_string("192.168.95.11"), 80}, io_service}));
  cameras.insert(std::make_pair("Cafofinho", eh::device::camera::dahua_ip
                                {{boost::asio::ip::address::from_string("192.168.95.12"), 80}, io_service}));
  cameras.insert(std::make_pair("Garagem", eh::device::camera::dahua_ip
                                {{boost::asio::ip::address::from_string("192.168.95.13"), 80}, io_service}));
  cameras.insert(std::make_pair("Jardim Inverno", eh::device::camera::dahua_ip
                                {{boost::asio::ip::address::from_string("192.168.95.14"), 80}, io_service}));
  
  luabind::module(L, "avail_devices")
  [
     // luabind::def("lg_rs232",
     //              luabind::tag_function<luabind::object(std::string, std::string, luabind::object)>
     //              ([&] (std::string name, std::string device, luabind::object function) -> luabind::object
     //              {
     //                auto iterator = lg_rs232s.emplace
     //                  (name, eh::device::lg_rs232{io_service, device}).first;
     //                luabind::object lg_obj(L, eh::devices::lua::device(iterator->second));
     //                iterator->second.watch(std::bind(&::callback_function, L, lg_obj, std::placeholders::_1, std::placeholders::_2
     //                                                 , function));
     //                return lg_obj;
     //              }))
     //,
   luabind::def("milight"
                , luabind::tag_function<luabind::object(std::string, std::string)>
                  ([&] (std::string name, std::string hostname) -> luabind::object
                  {
                    auto iterator = device_map.emplace
                     (name, eh::device::output::milight_device{io_service, {boost::asio::ip::address::from_string(hostname), 8899}}).first;
                    luabind::object device(L, eh::devices::lua::device(iterator->second));
                    // iterator->second.watch(std::bind(&::callback_function, L, lg_obj, std::placeholders::_1, std::placeholders::_2
                    //                                  , function));
                    return device;
                  }))
   , luabind::def("harmony"
                  , luabind::tag_function<luabind::object(std::string /*name*/, std::string /*hostname */, int /*port*/, std::string /* hubid */, std::string /* client_id */)>
                  ([&] (std::string name, std::string hostname, int port, std::string hubid, std::string clientid) -> luabind::object
                  {
                    auto iterator = harmony_devices.emplace
                      (name, eh::device::harmony_device{io_service, {boost::asio::ip::address::from_string(hostname), static_cast<unsigned short>(port)}, hubid, clientid}).first;
                    luabind::object device(L, eh::devices::lua::harmony_device(iterator->second));
                    // iterator->second.watch(std::bind(&::callback_function, L, lg_obj, std::placeholders::_1, std::placeholders::_2
                    //                                  , function));
                    return device;
                  }))

   //   luabind::def("lg",
   //                luabind::tag_function<luabind::object(std::string, std::string, std::string, luabind::object)>
   //                ([&] (std::string name, std::string hostname, std::string pass
   //                      , luabind::object function) -> luabind::object
   //                {
   //                  auto iterator = lgs.emplace
   //                    (name, eh::device::lg_ip{io_service, hostname, pass}).first;
   //                  luabind::object lg_obj(L, eh::devices::lua::device(iterator->second));
   //                  iterator->second.watch(std::bind(&::callback_function, L, lg_obj, std::placeholders::_1, std::placeholders::_2
   //                                                   , function));
   //                  return lg_obj;
   //                }))
   // , luabind::def("denon",
   //                luabind::tag_function<luabind::object(std::string, std::string, luabind::object)>
   //                ([&] (std::string name, std::string hostname, luabind::object function) -> luabind::object
   //                {
   //                  auto iterator = denons.emplace
   //                    (name, eh::device::denon_ip{io_service, hostname}).first;
   //                  luabind::object denon_obj(L, eh::devices::lua::device(iterator->second));
   //                  iterator->second.watch(std::bind(&::callback_function, L, denon_obj, std::placeholders::_1
   //                                                   , std::placeholders::_2, function));
   //                  return denon_obj;
   //                }))
   // , luabind::def("lg_power_on",
   //                luabind::tag_function<luabind::object(std::string, std::string, std::string, std::string, std::string
   //                                                      , luabind::object)>
   //                ([&] (std::string name, std::string hostname, std::string device
   //                      , std::string email, std::string password, luabind::object function) -> luabind::object
   //                {
   //                  auto iterator = harmony_devices.emplace
   //                    (name, eh::device::harmony_device{io_service, hostname, device, email, password}).first;
   //                  luabind::object harmony_obj(L, eh::devices::lua::device(iterator->second));
   //                  iterator->second.watch(std::bind(&::callback_function, L, harmony_obj, std::placeholders::_1
   //                                                   , std::placeholders::_2, function));
   //                  return harmony_obj;
   //                }))
   // , luabind::def("roku_ip",
   //                luabind::tag_function<luabind::object(std::string, std::string, unsigned short, luabind::object
   //                                                      , std::string)>
   //                ([&] (std::string name, std::string listen_ip, unsigned short port
   //                      , luabind::object function, std::string serial) -> luabind::object
   //                {
   //                  auto iterator = roku_ips.emplace
   //                    (name, eh::device::roku_ip{io_service, listen_ip, port, ssdp_server, serial}).first;
   //                  luabind::object roku_obj(L, eh::devices::lua::input(iterator->second));
   //                  iterator->second.watch(std::bind(&::callback_function, L, roku_obj, std::placeholders::_1
   //                                                   , std::placeholders::_2, function));
   //                  return roku_obj;
   //                }))
  ];

  if(luaL_loadfile(L, "lua/setup.lua"))
  {
    std::cout << "Error loading lua setup.lua" << std::endl;
    std::cout << "Error: " << lua_tostring(L, -1) << std::endl;
    return -1;
  }

  if(lua_pcall(L, 0, 0, 0))
  {
    std::cout << "Error running lua setup.lua" << std::endl;

    std::cout << "Error: " << lua_tostring(L, -1) << std::endl;
    
    return -1;
  }
  
  struct {
    const char* msg;
    overload function;
  } calaosapi[] =
      {
        {"login", [&] (auto data, auto msg_id, auto& client)
         {
           std::cout << "login" << std::endl;

           nlohmann::json json;
           json["msg"] = "login";
           //json["msg_id"] = data["msg_id"];
           // if(data["login"] == "")
           //   json["data"]["success"] = "false";
           // else
             json["data"]["success"] = "true";

           std::cout << "to send" << json.dump() << std::endl;
           client.write(json.dump());
         }}
        , {"get_home", [&] (auto data, auto msg_id, auto& client)
           {
             eh::calaos::get_home(data, msg_id, client, L);
           }}
        , {"set_state", [&] (auto data, auto msg_id, auto& client)
           {
             // answer {"msg":"event","data":{"event_raw":"io_changed id:io_1 state:%23000000 state_int:0","type":"3","type_str":"io_changed","data":{"id":"io_1","state":"#000000","state_int":"0"}}}
             std::cout << "set_state " << data["value"] << std::endl;
             // nlohmann::json

             std::string value = boost::algorithm::trim_all_copy_if
             (data["value"], [] (auto c)
               {
                 return c == '"';
               });

             std::string cmd;
             if(boost::algorithm::starts_with(value, "set "))
               {
                 cmd = std::string(data["value"]).substr(4);
               }
             else if(value == "true")
               {
                 cmd = "on";
               }
             else if(value == "false")
               {
                 cmd = "off";
               }

             auto iterator = device_map.find(data["id"]);
             if(iterator != device_map.end())
               {
                 std::cout << "command setting " << cmd << std::endl;
                 iterator->second.send_command(cmd, {}
                    , [&,msg_id] (auto ec, auto new_state)
                    {
             //          std::string raw = "io_changed id:";
             //          raw += data["id"];
             //          raw += " state:%23000000 state_int:0";
             //          nlohmann::json json =
             //            {
             //              {"msg", "event"}
             //              , {"data"
             //                 , {
             //                     {"id", data["id"]}
             //                     , {"event_raw", raw}
             //                     , {"type", "3"}
             //                     , {"type_str", "io_changed"}
             //                     , {"data"
             //                        , {
             //                             {"id", data["id"]}
             //                             , {"state", "#000000"}
             //                             , {"state_int", "0"}}}}}
             //            };
                      client.write("{\"msg\":\"set_state\",\"msg_id\":\"" +
                                   msg_id + "\",\"data\":{\"success\":\"true\"}}");
                    });

                 // should answer something
               }
           }}
        , {"autoscenario", [&] (auto data, auto msg_id, auto& client)
         {
           nlohmann::json json =
           {
             {"msg", "autoscenario"}
             , {"msg_id", msg_id}
             , {"data",
                {
                  {"scenarios", nlohmann::json::array()}
                }}
           };
           client.write(json.dump());
         }}
        , {"set_state", [&] (auto data, auto msg_id, auto& client)
         {
           luabind::object devices(luabind::globals(L)["devices"]);
           if(devices)
           {
             auto device = devices[get_string_value(data["id"])];
             if(device)
             {
               eh::calaos::lua_device lua_device(device);
               lua_device.send_command("command", {});
             }
           }             
         }}
        , {"get_picture", [&] (auto data, auto msg_id, auto& client)
         {
           // std::cout << "get_picture with data " << data  << std::endl;
           std::cout << "get_picture of " << data.size()  << std::endl;
           auto id = data.find("id");
           // if(!data["id"])
           if(id != data.end())
           {
             auto iterator = cameras.find(get_string_value(*id));
             if(iterator != cameras.end())
             {
               boost::unique_future<eh::device::camera::snapshot_image> image
                 = iterator->second.snapshot();

               image.then([&client] (boost::unique_future<eh::device::camera::snapshot_image> f)
                          {
                            std::cout << "received picture, answering" << std::endl;
                            eh::calaos::http_communication::resp_type resp {beast::http::status::ok, 11};
                            resp.reason("OK");
                            std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                            
                            eh::device::camera::snapshot_image image;
                            try
                            {
                              image = f.get();
                            }
                            catch(...)
                            {
                              std::cout << "failed f.get()" << std::endl;
                            }
                            std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                            resp.set("Content-Type", image.content_type);
                            resp.set("Content-Length", image.buffer.size());
                            resp.body.insert(resp.body.begin(), image.buffer.begin(), image.buffer.end());
                            std::cout << __FILE__ << ":" << __LINE__ << std::endl;
           
                            write_resp w(resp);
                            std::cout << __FILE__ << ":" << __LINE__ << std::endl;

                            try
                            {
                              std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                              client.socket.apply_visitor(w);
                              std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                              /*[&] (auto& socket)
                                { beast::http::write(socket, resp); });*/
                              // client.close();
                            }
                            catch(...)
                            {
                              std::cout << "failed" << std::endl;
                            }
                            std::cout << "finished" << std::endl;
                          });
             }
             else
               client.close();
           }
           else
             client.close();
         }}
      };

  auto calaosapi_demuxer = [&] (std::string const& json, auto& client)
       {
         std::cout << "api with json " << json << " size " << json.size() << std::endl;

         auto j = nlohmann::json::parse(json);
         auto msg = j["msg"].is_null() ? j["type"] : j["msg"];
         bool found = false;
         for(auto&& api : calaosapi)
           {
             if(api.msg == msg)
               {
                 found = true;
                 std::string msg_id = j["msg_id"].is_null() ? "" : j["msg_id"].get<std::string>();
                 if(j["data"].is_null())
                   api.function(j, msg_id, client);
                 else
                   api.function(j["data"], msg_id, client);
               }
           }
         if(!found)
           std::cout << "Unknown command " << msg << std::endl;
       };

  auto calaosapi_url_demuxer = [&] (std::map<std::string, std::string> const& map, auto& client)
       {
         std::cout << "api with map " << map.size() << std::endl;

         for(auto&& api : calaosapi)
         {
           auto iterator = map.find("type");
           if(iterator != map.end() && iterator->second == api.msg)
           {
             api.function(map, "", client);
           }
         }
       };
  
  eh::calaos::http_communication communication_api
    ({boost::asio::ip::address::from_string("0.0.0.0"), 5454}
     , {boost::asio::ip::address::from_string("0.0.0.0"), 4444}
     , calaosapi_demuxer
     , calaosapi_url_demuxer
     , io_service);
  communication_api.accept();
  // //eh::server::ssdp ssdp_server(io_service, 1900);
  

  // std::cout << "waiting requests" << std::endl;

  boost::asio::deadline_timer timer(io_service);
  timer.expires_from_now(boost::posix_time::seconds(20));

  timer.async_wait([&] (auto&&)
                   {
                     communication_api.broadcast_event
                       (
                        //"{\"msg\":\"event\", \"data\": {\"type_str\":\"touchscreen_camera_request\", \"data\": {\"id\": \"Fundos\"}, \"event_raw\": \"blablablablal\"} }\r\n"
                        "{\"msg\":\"event\", \"data\": {\"type_str\":\"io_changed\", \"data\": {\"id\": \"net\"}, \"state\":\"true\", \"event_raw\": \"blablablablal\"} }\r\n"
                       );
                   });
  
  io_service.run();
}
