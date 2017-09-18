// Copyright Felipe Magno de Almeida 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXPERT_DEVICES_CAMERAS_CAMERA_BASE_HPP
#define EXPERT_DEVICES_CAMERAS_CAMERA_BASE_HPP

namespace eh { namespace device { namespace camera {

struct snapshot_image
{
  std::vector<char> buffer;
  std::string content_type;
};
      
struct camera_base
{
  virtual snapshot_image snapshot()  const = 0;
};
      
} } }

#endif
