/*
This code is licensed under the GNU GPL v3.
Copyright: (C) 2024 Mattis DALLEAU
*/

#pragma once

#include "HelNet/client/unwrapped.hpp"
#include "HelNet/client/wrapper.hpp"

namespace hl
{
namespace net
{
    using udp_client_unwrapped = base_client_unwrapped<boost::asio::ip::udp>;
    using udp_client = client_wrapper<udp_client_unwrapped>;
}
}