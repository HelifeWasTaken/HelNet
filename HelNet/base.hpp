/*
This code is licensed under the GNU GPL v3.
Copyright: (C) 2024 Mattis DALLEAU
*/

#pragma once

#include <map>
#include <queue>
#include <array>

#include <hl/silva/collections/stdint.hpp>
#include <hl/silva/collections/meta.hpp>

#include <limits>
#include <atomic>
#include <memory>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>

namespace hl
{
namespace net
{

#ifndef HL_NET_BUFFER_SIZE
    #define HL_NET_BUFFER_SIZE 1024
#endif

static_assert(HL_NET_BUFFER_SIZE > 0, "HL_NET_BUFFER_SIZE must be greater than 0");

#ifndef HL_NET_BUFFER_SIZE_STATIC_ASSERT_DISABLED
    static_assert(HL_NET_BUFFER_SIZE <= 65536, "HL_NET_BUFFER_SIZE must be less than or equal to 65536");
#endif

#if __cplusplus >= 201703L
    using byte = std::byte;
#else
    using byte = u8;
#endif

#if __cplusplus >= 201103L
    #define HL_NET_CONSTEXPR constexpr
#else
    #define HL_NET_CONSTEXPR const
#endif

#if __cplusplus >= 201703L
    #define HL_NET_IF_CONSTEXPR if constexpr
#else
    #define HL_NET_IF_CONSTEXPR if
#endif

    #define HL_NET_STATIC_CONSTEXPR HL_NET_CONSTEXPR static

    using port_t = u16;

    using buffer_t = std::array<byte, HL_NET_BUFFER_SIZE>;
    using shared_buffer_t = std::shared_ptr<buffer_t>;

    using client_id_t = std::uint64_t;
    using atomic_client_id = std::atomic<client_id_t>;

    HL_NET_STATIC_CONSTEXPR client_id_t INVALID_CLIENT_ID  = std::numeric_limits<client_id_t>::max();
    HL_NET_STATIC_CONSTEXPR client_id_t BASE_CLIENT_ID     = INVALID_CLIENT_ID + 1; // Should be 0

    HL_NET_STATIC_CONSTEXPR port_t MAX_PORT = std::numeric_limits<port_t>::max();
    HL_NET_STATIC_CONSTEXPR port_t MIN_PORT = std::numeric_limits<port_t>::min();

    using unhealthy_connexions_t = std::queue<client_id_t>;

    template<typename CallbackTypes>
    using callback_layer_register_t = std::map<std::string, CallbackTypes>;

#ifdef SOMAXCONN
    HL_NET_STATIC_CONSTEXPR uint64_t MAX_CONNECTIONS = SOMAXCONN;
#else
    HL_NET_STATIC_CONSTEXPR uint64_t MAX_CONNECTIONS = 4096; // Standard on most systems
#endif

    static const char *DEFAULT_REGISTER_LAYER = "__hl_net_default_layer__";

    #define _HL_INTERNAL_LOCK_GUARD_WHEN_TRUE(CONDITION, LOCK_NAME, MUTEX, CODE) \
        do { HL_NET_IF_CONSTEXPR (CONDITION) { std::lock_guard<std::mutex> LOCK_NAME(MUTEX); CODE } else { CODE } } while (0)

    static inline shared_buffer_t make_shared_buffer()
    {
        buffer_t *buffer = new buffer_t();
        std::fill(buffer->begin(), buffer->end(), byte(0));
        return shared_buffer_t(buffer);
    }

    static inline shared_buffer_t make_shared_buffer(const byte *data, size_t size)
    {
        shared_buffer_t shared_buffer(new buffer_t());
        std::copy(data, data + std::min(size, shared_buffer->size()), shared_buffer->begin());
        return shared_buffer;
    }

    static inline shared_buffer_t make_shared_buffer(const shared_buffer_t &data)
    {
        return make_shared_buffer(data->data(), data->size());
    }

    static inline shared_buffer_t make_shared_buffer(const shared_buffer_t &data, const size_t size)
    {
        return make_shared_buffer(data->data(), size);
    }

    static inline shared_buffer_t make_shared_buffer(const buffer_t &data, const size_t size)
    {
        return make_shared_buffer(data.data(), size);
    }

    static inline shared_buffer_t make_shared_buffer(const buffer_t &data)
    {
        return make_shared_buffer(data.data(), data.size());
    }

}
}