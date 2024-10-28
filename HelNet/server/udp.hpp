/*
This code is licensed under the GNU GPL v3.
Copyright: (C) 2024 Mattis DALLEAU
*/

#pragma once

#include "HelNet/server/udp/server_unwrapped.hpp"
#include "HelNet/server/wrapper.hpp"

namespace hl
{
namespace net
{
    using udp_server = server_wrapper<udp_server_unwrapped>;
}
}