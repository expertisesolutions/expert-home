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
  luabind::object devices(luabind::globals(L)["devices"]);
  for(luabind::iterator iterator (devices), end; iterator != end; ++iterator)
  {
    std::cout << "looping" << std::endl;
    lua_device lua_device(*iterator);
    std::string key;
    items.push_back
      (nlohmann::json
       ({
         {"id", luabind::object_cast<std::string>(iterator.key())}
        , {"name", luabind::object_cast<std::string>(iterator.key())}
        , {"type", lua_device.type()}
        , {"var_type", lua_device.var_type()}
        , {"visible", "true"}
        , {"state", lua_device.state()}
        , {"io_type", lua_device.io_type()}
        , {"gui_type", lua_device.gui_type()}
       }));
  }

  nlohmann::json json =
    {
      {"msg", "get_home"}
      , {"msg_id", msg_id}
      , {"data"
         , {
             {"home", nlohmann::json::array
              ({{
                  {"type", "salon"}
                  , {"name", "sala"}
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
                        items
                        }
                }})}
             , {"cameras",
                nlohmann::json::array
                ({{
                    {"id", "camera_id"}
                    , {"name", "camera_name"}
                    , {"type", "string"}
                    , {"ptz", "true"}
                }})}
             , {"audio", nlohmann::json::array()}
        }}
    };
  client.write(json.dump());
}
    
} }

#endif

