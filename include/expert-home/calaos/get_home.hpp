// Copyright Felipe Magno de Almeida 2016-2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_HOME_CALAOS_GET_HOME_HPP
#define EXPERT_HOME_CALAOS_GET_HOME_HPP

#include <expert-home/calaos/http_communication.hpp>
#include <expert-home/calaos/lua_device.hpp>
#include <json.hpp>

namespace eh { namespace calaos {

void get_home(nlohmann::json data, std::string msg_id, http_communication::client& client, lua_State* L);

} }

#endif

