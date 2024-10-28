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
    class base_abstract_client_unwrapped;

    using client_t                              = std::shared_ptr<base_abstract_client_unwrapped>;

    using client_on_connect_callback            = std::function<void(client_t client)>;
    using client_on_disconnect_callback         = std::function<void(void)>;
    using client_on_disconnect_error_callback   = std::function<void(const boost::system::error_code &ec)>;
    using client_on_receive_callback            = std::function<void(client_t client, shared_buffer_t buffer_copy, const size_t recv_bytes)>;
    using client_on_receive_error_callback      = std::function<void(client_t client, shared_buffer_t buffer_copy, const boost::system::error_code ec, const size_t recv_bytes)>;
    using client_on_sent_callback               = std::function<void(client_t client, const size_t sent_bytes)>;
    using client_on_send_error_callback         = std::function<void(client_t client, const boost::system::error_code ec, const size_t sent_bytes)>;

    #define HL_NET_CLIENT_ON_CONNECT(CLIENT) [](client_t CLIENT)
    #define HL_NET_CLIENT_ON_CONNECT_CAPTURE(CLIENT, ...) [__VA_ARGS__](client_t CLIENT)
    #define HL_NET_CLIENT_ON_DISCONNECT() []() 
    #define HL_NET_CLIENT_ON_DISCONNECT_CAPTURE(...) [__VA_ARGS__]()
    #define HL_NET_CLIENT_ON_DISCONNECT_ERROR(EC) [](const boost::system::error_code &EC)
    #define HL_NET_CLIENT_ON_DISCONNECT_ERROR_CAPTURE(EC, ...) [__VA_ARGS__](const boost::system::error_code &EC)
    #define HL_NET_CLIENT_ON_RECEIVE(CLIENT, BUFFER_COPY, RECV_BYTES) [](client_t CLIENT, shared_buffer_t BUFFER_COPY, const size_t RECV_BYTES) 
    #define HL_NET_CLIENT_ON_RECEIVE_CAPTURE(CLIENT, BUFFER_COPY, RECV_BYTES, ...) [__VA_ARGS__](client_t CLIENT, shared_buffer_t BUFFER_COPY, const size_t RECV_BYTES)
    #define HL_NET_CLIENT_ON_RECEIVE_ERROR(CLIENT, BUFFER_COPY, EC, RECV_BYTES) [](client_t CLIENT, shared_buffer_t BUFFER_COPY, const boost::system::error_code EC, const size_t RECV_BYTES)
    #define HL_NET_CLIENT_ON_RECEIVE_ERROR_CAPTURE(CLIENT, BUFFER_COPY, EC, RECV_BYTES, ...) [__VA_ARGS__](client_t CLIENT, shared_buffer_t BUFFER_COPY, const boost::system::error_code EC, const size_t RECV_BYTES)
    #define HL_NET_CLIENT_ON_SENT(CLIENT, SENT_BYTES) [](client_t CLIENT, const size_t SENT_BYTES)
    #define HL_NET_CLIENT_ON_SENT_CAPTURE(CLIENT, SENT_BYTES, ...) [__VA_ARGS__](client_t CLIENT, const size_t SENT_BYTES)
    #define HL_NET_CLIENT_ON_SEND_ERROR(CLIENT, EC, SENT_BYTES) [](client_t CLIENT, const boost::system::error_code EC, const size_t SENT_BYTES)
    #define HL_NET_CLIENT_ON_SEND_ERROR_CAPTURE(CLIENT, EC, SENT_BYTES, ...) [__VA_ARGS__](client_t CLIENT, const boost::system::error_code EC, const size_t SENT_BYTES)

    struct client_callbacks final {
        client_on_connect_callback          on_connect_callback = nullptr;
        bool                                on_connect_is_async = false;

        client_on_disconnect_callback       on_disconnect_callback = nullptr;
        bool                                on_disconnect_is_async = false;

        client_on_disconnect_error_callback on_disconnect_error_callback = nullptr;
        bool                                on_disconnect_error_is_async = false;

        client_on_receive_callback          on_receive_callback = nullptr;
        bool                                on_receive_is_async = false;

        client_on_receive_error_callback    on_receive_error_callback = nullptr;
        bool                                on_receive_error_is_async = false;

        client_on_sent_callback             on_sent_callback = nullptr;
        bool                                on_sent_is_async = false;

        client_on_send_error_callback       on_send_error_callback = nullptr;
        bool                                on_send_error_is_async = false;
    };

    class client_callback_register final : public hl::silva::collections::meta::NonCopyMoveable
    {
    private:
        hl::silva::collections::threads::basic_pool_async m_pool;

    private:
        callback_layer_register_t<client_callbacks> m_callbacks;
        mutable std::mutex m_callbacks_mutex;
        std::function<client_t(void)> m_get_sharable;

    public:
        _HL_INTERNAL_CALLBACK_IMPL_BASE(client_callbacks, m_pool, m_callbacks_mutex, m_callbacks);


#define _HL_INTERNAL_CLIENT_CALLBACK_REGISTER_IMPL(NAME) \
        _HL_INTERNAL_CALLBACK_REGISTER_IMPL(NAME, client, m_callbacks, m_pool, m_callbacks_mutex, m_get_sharable)

#define _HL_INTERNAL_CLIENT_CALLBACK_REGISTER_IMPL_NO_SHARABLE(NAME) \
        _HL_INTERNAL_CALLBACK_REGISTER_IMPL_NO_SHARABLE(NAME, client, m_callbacks, m_pool, m_callbacks_mutex)

        _HL_INTERNAL_CLIENT_CALLBACK_REGISTER_IMPL(on_connect);
        _HL_INTERNAL_CLIENT_CALLBACK_REGISTER_IMPL_NO_SHARABLE(on_disconnect);
        _HL_INTERNAL_CLIENT_CALLBACK_REGISTER_IMPL_NO_SHARABLE(on_disconnect_error);
        _HL_INTERNAL_CLIENT_CALLBACK_REGISTER_IMPL(on_receive);
        _HL_INTERNAL_CLIENT_CALLBACK_REGISTER_IMPL(on_receive_error);
        _HL_INTERNAL_CLIENT_CALLBACK_REGISTER_IMPL(on_sent);
        _HL_INTERNAL_CLIENT_CALLBACK_REGISTER_IMPL(on_send_error);

#undef _HL_INTERNAL_CLIENT_CALLBACK_REGISTER_IMPL
#undef _HL_INTERNAL_CLIENT_CALLBACK_REGISTER_IMPL_NO_SHARABLE

    public:
        client_callback_register(std::function<client_t(void)> get_sharable)
            : m_pool(true)
            , m_callbacks()
            , m_callbacks_mutex()
            , m_get_sharable(get_sharable)
        {}

        ~client_callback_register() = default;
    };

}
}