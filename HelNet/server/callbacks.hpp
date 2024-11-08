/*
This code is licensed under the GNU GPL v3.
Copyright: (C) 2024 Mattis DALLEAU
*/

#pragma once

#include "HelNet/base.hpp"
#include "HelNet/logger.hpp"
#include "HelNet/defines.hpp"

#include <hl/silva/collections/threads/basic_pool_async.hpp>
#include <boost/system/error_code.hpp>

namespace hl
{
namespace net
{
    class base_abstract_server_unwrapped;
    class base_abstract_connection_unwrapped;

    using server_t                                  = boost::shared_ptr<base_abstract_server_unwrapped>;
    using connection_t                              = boost::shared_ptr<base_abstract_connection_unwrapped>;

    using server_on_start_success_callback          = std::function<void(server_t server)>;

    using server_on_stop_success_callback           = std::function<void()>;
    using server_on_stop_error_callback             = std::function<void(const boost::system::error_code ec)>;

    using server_on_connection_callback             = std::function<void(server_t server, connection_t client)>;
    using server_on_connection_error_callback       = std::function<void(server_t server, const boost::system::error_code ec)>;

    using server_on_disconnection_callback          = std::function<void(server_t server, const client_id_t& client)>;
    using server_on_disconnection_error_callback    = std::function<void(server_t server, const boost::system::error_code ec)>;

    using server_on_sent_callback                   = std::function<void(server_t server, connection_t client, const size_t sent_bytes)>;
    using server_on_send_error_callback             = std::function<void(server_t server, connection_t client, const boost::system::error_code ec, const size_t sent_bytes)>;

    using server_on_receive_callback                = std::function<void(server_t server, connection_t client, shared_buffer_t buffer_copy, const size_t recv_bytes)>;
    using server_on_receive_error_callback          = std::function<void(server_t server, connection_t client, shared_buffer_t buffer_copy, const boost::system::error_code ec, const size_t recv_bytes)>;

    #define HL_NET_SERVER_ON_START(SERVER) [](server_t SERVER)
    #define HL_NET_SERVER_ON_START_CAPTURE(SERVER, ...) [__VA_ARGS__](server_t SERVER)
    #define HL_NET_SERVER_ON_STOP() []()
    #define HL_NET_SERVER_ON_STOP_CAPTURE(...) [__VA_ARGS__]()
    #define HL_NET_SERVER_ON_STOP_ERROR(EC) [](const boost::system::error_code &EC)
    #define HL_NET_SERVER_ON_STOP_ERROR_CAPTURE(EC, ...) [__VA_ARGS__](const boost::system::error_code &EC)
    #define HL_NET_SERVER_ON_CONNECTION(SERVER, CLIENT) [](server_t SERVER, connection_t CLIENT)
    #define HL_NET_SERVER_ON_CONNECTION_CAPTURE(SERVER, CLIENT, ...) [__VA_ARGS__](server_t SERVER, connection_t CLIENT)
    #define HL_NET_SERVER_ON_CONNECTION_ERROR(SERVER, EC) [](server_t SERVER, const boost::system::error_code &EC)
    #define HL_NET_SERVER_ON_CONNECTION_ERROR_CAPTURE(SERVER, EC, ...) [__VA_ARGS__](server_t SERVER, const boost::system::error_code &EC)
    #define HL_NET_SERVER_ON_DISCONNECTION(SERVER, CLIENT_ID) [](server_t SERVER, const client_id_t &CLIENT_ID)
    #define HL_NET_SERVER_ON_DISCONNECTION_CAPTURE(SERVER, CLIENT_ID, ...) [__VA_ARGS__](server_t SERVER, const client_id_t &CLIENT_ID)
    #define HL_NET_SERVER_ON_DISCONNECTION_ERROR(SERVER, EC) [](server_t SERVER, const boost::system::error_code &EC)
    #define HL_NET_SERVER_ON_DISCONNECTION_ERROR_CAPTURE(SERVER, EC, ...) [__VA_ARGS__](server_t SERVER, const boost::system::error_code &EC)
    #define HL_NET_SERVER_ON_SENT(SERVER, CLIENT, SENT_BYTES) [](server_t SERVER, connection_t CLIENT, const size_t SENT_BYTES)
    #define HL_NET_SERVER_ON_SENT_CAPTURE(SERVER, CLIENT, SENT_BYTES, ...) [__VA_ARGS__](server_t SERVER, connection_t CLIENT, const size_t SENT_BYTES)
    #define HL_NET_SERVER_ON_SEND_ERROR(SERVER, CLIENT, EC, SENT_BYTES) [](server_t SERVER, connection_t CLIENT, const boost::system::error_code &EC, const size_t SENT_BYTES)
    #define HL_NET_SERVER_ON_SEND_ERROR_CAPTURE(SERVER, CLIENT, EC, SENT_BYTES, ...) [__VA_ARGS__](server_t SERVER, connection_t CLIENT, const boost::system::error_code &EC, const size_t SENT_BYTES)
    #define HL_NET_SERVER_ON_RECEIVE(SERVER, CLIENT, BUFFER_COPY, RECV_BYTES) [](server_t SERVER, connection_t CLIENT, shared_buffer_t BUFFER_COPY, const size_t RECV_BYTES)
    #define HL_NET_SERVER_ON_RECEIVE_CAPTURE(SERVER, CLIENT, BUFFER_COPY, RECV_BYTES, ...) [__VA_ARGS__](server_t SERVER, connection_t CLIENT, shared_buffer_t BUFFER_COPY, const size_t RECV_BYTES)
    #define HL_NET_SERVER_ON_RECEIVE_ERROR(SERVER, CLIENT, BUFFER_COPY, EC, RECV_BYTES) [](server_t SERVER, connection_t CLIENT, shared_buffer_t BUFFER_COPY, const boost::system::error_code &EC, const size_t RECV_BYTES)
    #define HL_NET_SERVER_ON_RECEIVE_ERROR_CAPTURE(SERVER, CLIENT, BUFFER_COPY, EC, RECV_BYTES, ...) [__VA_ARGS__](server_t SERVER, connection_t CLIENT, shared_buffer_t BUFFER_COPY, const boost::system::error_code &EC, const size_t RECV_BYTES)

    struct server_callbacks final {
        server_on_start_success_callback    on_start_success_callback = nullptr;
        bool                                on_start_success_is_async = false;

        server_on_stop_success_callback     on_stop_success_callback = nullptr;
        bool                                on_stop_success_is_async = false;

        server_on_stop_error_callback       on_stop_error_callback = nullptr;
        bool                                on_stop_error_is_async = false;

        server_on_connection_callback       on_connection_callback = nullptr;
        bool                                on_connection_is_async = false;

        server_on_connection_error_callback on_connection_error_callback = nullptr;
        bool                                on_connection_error_is_async = false;

        server_on_disconnection_callback    on_disconnection_callback = nullptr;
        bool                                on_disconnection_is_async = false;

        server_on_disconnection_error_callback on_disconnection_error_callback = nullptr;
        bool                                   on_disconnection_error_is_async = false;

        server_on_sent_callback             on_sent_callback = nullptr;
        bool                                on_sent_is_async = false;

        server_on_send_error_callback       on_send_error_callback = nullptr;
        bool                                on_send_error_is_async = false;

        server_on_receive_callback          on_receive_callback = nullptr;
        bool                                on_receive_is_async = false;

        server_on_receive_error_callback    on_receive_error_callback = nullptr;
        bool                                on_receive_error_is_async = false;

        server_callbacks() = default;
        ~server_callbacks() = default;
    };

    class server_callback_register final : public hl::silva::collections::meta::NonCopyMoveable
    {
    private:
        hl::silva::collections::threads::basic_pool_async m_pool;
        
    private:
        callback_layer_register_t<server_callbacks> m_callbacks;
        mutable std::mutex m_mutex;
        std::function<server_t(void)> m_get_sharable;

    public:
        _HL_INTERNAL_CALLBACK_IMPL_BASE(server_callbacks, m_pool, m_mutex, m_callbacks);

#define _HL_INTERNAL_SERVER_CALLBACK_REGISTER_IMPL(NAME) \
        _HL_INTERNAL_CALLBACK_REGISTER_IMPL(NAME, server, m_callbacks, m_pool, m_mutex, m_get_sharable)

#define _HL_INTERNAL_SERVER_CALLBACK_REGISTER_IMPL_NO_SHARABLE(NAME) \
        _HL_INTERNAL_CALLBACK_REGISTER_IMPL_NO_SHARABLE(NAME, server, m_callbacks, m_pool, m_mutex)

        _HL_INTERNAL_SERVER_CALLBACK_REGISTER_IMPL(on_start_success);
        _HL_INTERNAL_SERVER_CALLBACK_REGISTER_IMPL_NO_SHARABLE(on_stop_success);
        _HL_INTERNAL_SERVER_CALLBACK_REGISTER_IMPL_NO_SHARABLE(on_stop_error);
        _HL_INTERNAL_SERVER_CALLBACK_REGISTER_IMPL(on_connection);
        _HL_INTERNAL_SERVER_CALLBACK_REGISTER_IMPL(on_connection_error);
        _HL_INTERNAL_SERVER_CALLBACK_REGISTER_IMPL(on_disconnection);
        _HL_INTERNAL_SERVER_CALLBACK_REGISTER_IMPL(on_disconnection_error);
        _HL_INTERNAL_SERVER_CALLBACK_REGISTER_IMPL(on_sent);
        _HL_INTERNAL_SERVER_CALLBACK_REGISTER_IMPL(on_send_error);
        _HL_INTERNAL_SERVER_CALLBACK_REGISTER_IMPL(on_receive);
        _HL_INTERNAL_SERVER_CALLBACK_REGISTER_IMPL(on_receive_error);

#undef _HL_INTERNAL_SERVER_CALLBACK_REGISTER_IMPL

        server_callback_register(std::function<server_t(void)> get_sharable)
            : m_pool(false)
            , m_callbacks()
            , m_mutex()
            , m_get_sharable(get_sharable)
        {}

        ~server_callback_register() = default;
    };
}
}
