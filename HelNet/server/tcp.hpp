/*
This code is licensed under the GNU GPL v3.
Copyright: (C) 2024 Mattis DALLEAU
*/

#pragma once

#include "HelNet/server/tcp/server_unwrapped.hpp"
#include "HelNet/server/wrapper.hpp"

namespace hl
{
namespace net
{
    using tcp_server = server_wrapper<tcp_server_unwrapped>;
}
}