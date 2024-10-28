/*
This code is licensed under the GNU GPL v3.
Copyright: (C) 2024 Mattis DALLEAU
*/

#pragma once

// TODO: Documentation
// TODO: Logging for all callbacks set and call / call error
// TODO: Enforce use of (this) keyword in all classes for readability
// TODO: Change map, unordered_map and queue to use custom containers from hl::silva::collections::thread_safe instead of std library
// TODO: Troughly test again TCP and UDP clients and servers
// TODO: Timeout plugin (Specifically for UDP server connections)
// TODO: Should i stop using spdlog and make a custom logger to avoid depending on a library ?
// TODO: Modify hl::silva::collections so it compiles with c++14 or less (only compiles with c++17>= for now)
// TODO: Should i change the buffer_t to a std::vector<byte> instead of a std::array<byte, HL_NET_BUFFER_SIZE> ?

#include "./HelNet/client/tcp.hpp"
#include "./HelNet/client/udp.hpp"
#include "./HelNet/server/tcp.hpp"
#include "./HelNet/server/udp.hpp"