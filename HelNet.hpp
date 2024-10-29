/*
This code is licensed under the GNU GPL v3.
Copyright: (C) 2024 Mattis DALLEAU
*/

#pragma once

// TODO: Documentation
// TODO: Enforce use of (this) keyword in all classes for readability
// TODO: Troughly test again TCP and UDP clients and servers
// TODO: Should i stop using spdlog and make a custom logger to avoid depending on a library ?
// TODO: Should i change the buffer_t to a std::vector<byte> instead of a std::array<byte, HL_NET_BUFFER_SIZE> ?
// TODO: Placeholders for callbacks hl::net::placeholders

#include "HelNet/client/tcp.hpp"
#include "HelNet/client/udp.hpp"
#include "HelNet/server/tcp.hpp"
#include "HelNet/server/udp.hpp"