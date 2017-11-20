// Copyright Felipe Magno de Almeida 2016.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_HOME_CALAOS_GET_HOME_HPP
#define EXPERT_HOME_CALAOS_GET_HOME_HPP

#include <expert-home/calaos/http_communication.hpp>
#include <expert-home/calaos/lua_device.hpp>
#include <json.hpp>

namespace eh { namespace calaos {

void get_home(nlohmann::json data, std::string msg_id, http_communication::client& client, lua_State* L)
{
  std::cout << "get_home" << std::endl;

  std::vector<nlohmann::json> items;
  std::vector<nlohmann::json> room_items;
  luabind::object devices(luabind::globals(L)["devices"]);
  for(luabind::iterator iterator (devices), end; iterator != end; ++iterator)
  {
    std::cout << "looping" << std::endl;
    lua_device lua_device(*iterator);
    std::string key;
    std::string name;
    if(lua_device.has_name())
      name = lua_device.name();
    else
      name = luabind::object_cast<std::string>(iterator.key());
    items.push_back
      (nlohmann::json
       ({
         {"id", luabind::object_cast<std::string>(iterator.key())}
        , {"name", name}
        , {"type", lua_device.type()}
        , {"var_type", lua_device.var_type()}
        , {"visible", "true"}
        , {"state", lua_device.state()}
        , {"io_type", lua_device.io_type()}
        , {"gui_type", lua_device.gui_type()}
       }));
  }
  items.push_back
      (nlohmann::json
       ({
         {"id", "net"}
        , {"name", "Watch TV"}
        , {"type", "scenario"}
         //        , {"var_type", "bool"}
        , {"visible", "true"}
        , {"save", "false"}
        , {"gui_type", "scenario"}
        , {"state", "true"} // auto scenario
        , {"auto_scenario", "true"} // auto scenario
        , {"category", "light"} // auto scenario
        , {"rw", "true"} // auto scenario
        , {"io_type", "inout"}
       }));
  items.push_back
      (nlohmann::json
       ({
         {"id", "chrome"}
        , {"name", "Assistir Chromecast"}
        , {"type", "scenario"}
         //        , {"var_type", "bool"}
        , {"visible", "true"}
        , {"save", "false"}
        , {"gui_type", "scenario"}
        , {"state", "true"} // auto scenario
        , {"auto_scenario", "true"} // auto scenario
        , {"category", "light"} // auto scenario
        , {"rw", "true"} // auto scenario
        , {"io_type", "inout"}
       }));
  items.push_back
      (nlohmann::json
       ({
         {"id", "analogin"}
        , {"name", "analogin"}
        , {"gui_type", "analog_in"}
         //  , {"var_type", "bool"}
        , {"visible", "true"}
        , {"save", "false"}
        // , {"state", "true"} // auto scenario
        // , {"auto_scenario", "true"} // auto scenario
        // , {"category", "light"} // auto scenario
        , {"rw", "true"} // auto scenario
        , {"io_type", "inout"}
       }));
  items.push_back
      (nlohmann::json
       ({
         {"id", "testeee"}
        , {"name", "Testeee"}
        , {"gui_type", "audio_player"}
         //  , {"var_type", "bool"}
        , {"visible", "true"}
        , {"save", "false"}
        // , {"state", "true"} // Auto scenario
        // , {"auto_scenario", "true"} // auto scenario
        // , {"category", "light"} // auto scenario
        , {"rw", "true"} // auto scenario
        , {"io_type", "inout"}
       }));
  items.push_back
      (nlohmann::json
       ({
         {"id", "Temperatura"}
        , {"name", "Temperatura"}
        , {"gui_type", "temp"}
         //  , {"var_type", "bool"}
        , {"visible", "true"}
        , {"save", "false"}
        // , {"state", "true"} // Auto scenario
        // , {"auto_scenario", "true"} // auto scenario
        // , {"category", "light"} // auto scenario
        , {"rw", "true"} // auto scenario
        , {"io_type", "inout"}
       }));

  room_items.push_back
      (nlohmann::json
       ({
         {"id", "abajurrgb"}
        , {"name", "Abajur"}
        , {"gui_type", "light_rgb"}
         //  , {"var_type", "bool"}
        , {"visible", "true"}
        , {"save", "false"}
        , {"state", "true"} // auto scenario
        // , {"auto_scenario", "true"} // auto scenario
        // , {"category", "light"} // auto scenario
        // , {"rw", "true"} // auto scenario
        , {"io_type", "inout"}
       }));
  
  std::vector<nlohmann::json> cameras;
  cameras.push_back
    (nlohmann::json
     ({
       {"id", "Liz Cama"}
       , {"name", "Liz Cama"}
       , {"type", "string"}
       , {"ptz", "true"}
     }));
  cameras.push_back
    (nlohmann::json
     ({
       {"id", "Jantar Exterior"}
       , {"name", "Jantar Exterior"}
       , {"type", "string"}
       , {"ptz", "true"}
     }));
  cameras.push_back
    (nlohmann::json
     ({
       {"id", "Piscina"}
       , {"name", "Piscina"}
       , {"type", "string"}
       , {"ptz", "true"}
     }));
  cameras.push_back
    (nlohmann::json
     ({
       {"id", "Interfone"}
       , {"name", "Interfone"}
       , {"type", "string"}
       , {"ptz", "true"}
     }));
  cameras.push_back
    (nlohmann::json
     ({
       {"id", "Carro"}
       , {"name", "Carro"}
       , {"type", "string"}
       , {"ptz", "true"}
     }));
  cameras.push_back
    (nlohmann::json
     ({
       {"id", "Liz Microfone"}
       , {"name", "Liz Microfone"}
       , {"type", "string"}
       , {"ptz", "true"}
     }));
  //
  cameras.push_back
    (nlohmann::json
     ({
       {"id", "Sala Jantar"}
       , {"name", "Sala Jantar"}
       , {"type", "string"}
       , {"ptz", "true"}
     }));
  cameras.push_back
    (nlohmann::json
     ({
       {"id", "Poste"}
       , {"name", "Poste"}
       , {"type", "string"}
       , {"ptz", "true"}
     }));
  cameras.push_back
    (nlohmann::json
     ({
       {"id", "Brinquedoteca"}
       , {"name", "Brinquedoteca"}
       , {"type", "string"}
       , {"ptz", "true"}
     }));
  cameras.push_back
    (nlohmann::json
     ({
       {"id", "Fundos"}
       , {"name", "Fundos"}
       , {"type", "string"}
       , {"ptz", "true"}
     }));
  cameras.push_back
    (nlohmann::json
     ({
       {"id", "Cafofinho"}
       , {"name", "Cafofinho"}
       , {"type", "string"}
       , {"ptz", "true"}
     }));
  cameras.push_back
    (nlohmann::json
     ({
       {"id", "Garagem"}
       , {"name", "Garagem"}
       , {"type", "string"}
       , {"ptz", "true"}
     }));
  cameras.push_back
    (nlohmann::json
     ({
       {"id", "Jardim Inverno"}
       , {"name", "Jardim Inverno"}
       , {"type", "string"}
       , {"ptz", "true"}
     }));
  
  nlohmann::json json =
    {
      {"msg", "get_home"}
      , {"msg_id", msg_id}
      , {"data"
         , {
             {"home", nlohmann::json::array
              ({{
                  {"type", "salon"}
                  , {"name", "Special"}
                  , {"hits", 0}
                  , {"items", // nlohmann::json::array
                        // ({
                        //   {
                        //     {"id", "io_0"}
                        //     , {"name", "New IO2"}
                        //     , {"type", "MilightOutputLightRGB"}
                        //     , {"var_type", "string"}
                        //     , {"visible", "true"}
                        //     , {"gui_type", "light_rgb"}
                        //     , {"state", "0"}
                        //     , {"io_type", "output"}
                        //   }
                        // })
                        {{"inputs", items}
                          , {"outputs", items}}
                        }
                }
                , {
                  {"type", "salon"}
                  , {"name", "Quarto"}
                  , {"hits", 0}
                  , {"items", // nlohmann::json::array
                        // ({
                        //   {
                        //     {"id", "io_0"}
                        //     , {"name", "New IO2"}
                        //     , {"type", "MilightOutputLightRGB"}
                        //     , {"var_type", "string"}
                        //     , {"visible", "true"}
                        //     , {"gui_type", "light_rgb"}
                        //     , {"state", "0"}
                        //     , {"io_type", "output"}
                        //   }
                        // })
                        {{"inputs", room_items}
                          , {"outputs", room_items}}
                        }
                }
              })}
             , {"cameras", cameras}
             , {"audio",
                nlohmann::json::array
                ({
                  {
                    {"id", "some_player"}
                    , {"name", "some_player"}
                    , {"type", "slim"} // or Squeezebox?
                    , {"playlist", "true"}
                    , {"database", "true"}
                    , {"gui_type", "avreceiver"}
                    //, {"avr", "?"}
                  }
                })
             }
        }}
    };
  client.write(json.dump());
}
    
} }

#endif

