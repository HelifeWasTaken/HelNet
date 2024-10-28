/*
This code is licensed under the GNU GPL v3.
Copyright: (C) 2024 Mattis DALLEAU
*/

#pragma once

#include "unwrapped.hpp"
#include "wrapper.hpp"

namespace hl
{
namespace net
{
    using tcp_client_unwrapped = base_client_unwrapped<boost::asio::ip::tcp>;
    using tcp_client = client_wrapper<tcp_client_unwrapped>;
}
}