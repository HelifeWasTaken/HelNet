/*
This code is licensed under the GNU GPL v3.
Copyright: (C) 2024 Mattis DALLEAU
*/

#pragma once

#include "./udp/server_unwrapped.hpp"
#include "./wrapper.hpp"

namespace hl
{
namespace net
{
    using udp_server = server_wrapper<udp_server_unwrapped>;
}
}