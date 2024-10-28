/*
This code is licensed under the GNU GPL v3.
Copyright: (C) 2024 Mattis DALLEAU
*/

#pragma once

#include "./callbacks.hpp"
#include "./plugins.hpp"

namespace hl
{
namespace net
{
    template<class Protocol>
    class server_wrapper : public hl::silva::collections::meta::NonCopyMoveable
    {
    private:
        typename Protocol::shared_t m_shared_server;
        Protocol &m_server;

    public:
        server_wrapper()
            : m_shared_server(Protocol::make())
            , m_server(*m_shared_server)
        {
            HL_NET_LOG_DEBUG("Creating server wrapper for server: {}", m_server.get_alias());

            m_server.set_alias(fmt::format("{}({})", typeid(Protocol).name(), reinterpret_cast<void*>(&m_server)));

            server_callback_register& callbacks = m_server.callbacks_register();
 
#if __GNUC__ || __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
            callbacks.set_on_start_success(HL_NET_SERVER_ON_START(server) { HL_NET_LOG_INFO("Server started: {}", server->get_alias()); });
            callbacks.set_on_stop_success(HL_NET_SERVER_ON_STOP() { HL_NET_LOG_INFO("Server stopped"); });
            callbacks.set_on_stop_error(HL_NET_SERVER_ON_STOP_ERROR(ec) { HL_NET_LOG_ERROR("Server stop error: {}", ec.message()); });
            callbacks.set_on_connection(HL_NET_SERVER_ON_CONNECTION(server, client) { HL_NET_LOG_INFO("Server accepted connection: {} - {}", server->get_alias(), client->get_alias()); });
            callbacks.set_on_connection_error(HL_NET_SERVER_ON_CONNECTION_ERROR(server, ec) { HL_NET_LOG_ERROR("Server connection error: {} - {}", server->get_alias(), ec.message()); });
            callbacks.set_on_disconnection(HL_NET_SERVER_ON_DISCONNECTION(server, client_id) { HL_NET_LOG_INFO("Server disconnected: {} - {}", server->get_alias(), client_id); });
            callbacks.set_on_disconnection_error(HL_NET_SERVER_ON_DISCONNECTION_ERROR(server, ec) { HL_NET_LOG_ERROR("Server disconnection error: {} - {}", server->get_alias(), ec.message()); });
            callbacks.set_on_sent(HL_NET_SERVER_ON_SENT(server, client, sent_bytes) { HL_NET_LOG_INFO("Server sent: {} - {} - {}", server->get_alias(), client->get_alias(), sent_bytes); });
            callbacks.set_on_send_error(HL_NET_SERVER_ON_SEND_ERROR(server, client, ec, sent_bytes) { HL_NET_LOG_ERROR("Server send error: {} - {} - {} - {}", server->get_alias(), client->get_alias(), ec.message(), sent_bytes); });
            callbacks.set_on_receive(HL_NET_SERVER_ON_RECEIVE(server, client, buffer_copy, recv_bytes) { HL_NET_LOG_INFO("Server received: {} - {} - {}", server->get_alias(), client->get_alias(), recv_bytes); });
            callbacks.set_on_receive_error(HL_NET_SERVER_ON_RECEIVE_ERROR(server, client, buffer_copy, ec, recv_bytes) { HL_NET_LOG_ERROR("Server receive error: {} - {} - {} - {}", server->get_alias(), client->get_alias(), ec.message(), recv_bytes); });

#if __GNUC__ || __clang__
#pragma GCC diagnostic pop
#endif


            HL_NET_LOG_DEBUG("Created server wrapper for server: {}", m_server.get_alias());
        }

        ~server_wrapper()
        {
            HL_NET_LOG_DEBUG("Destroying server wrapper for server: {}", m_server.get_alias());
        }

        server_callback_register& callbacks_register()
        {
            return m_server.callbacks_register();
        }

        std::string get_alias() const
        {
            return m_server.get_alias();
        }

        void set_alias(const std::string &alias)
        {
            m_server.set_alias(alias);
        }

        bool start(const std::string &port)
        {
            return m_server.start(port);
        }

        bool stop()
        {
            return m_server.stop();
        }

        bool send(const client_id_t& client_id, const shared_buffer_t &buffer, const size_t &size)
        {
            return m_server.send(client_id, buffer, size);
        }

        bool disconnect(const client_id_t& client_id)
        {
            return m_server.disconnect(client_id);
        }

        bool send_bytes(const client_id_t& client_id, const void *data, const size_t &size)
        {
            return m_server.send_bytes(client_id, data, size);
        }

        bool send_string(const client_id_t& client_id, const std::string &str)
        {
            return m_server.send_string(client_id, str);
        }

        template<typename T>
        bool send_bytes(const client_id_t& client_id, const std::vector<T> &data)
        {
            return m_server.send_bytes(client_id, data);
        }

        bool healthy() const
        {
            return m_server.healthy();
        }

        bool should_exit() const
        {
            return m_server.should_exit();
        }

        bool request_stop()
        {
            return m_server.request_stop();
        }

        operator bool()
        {
            return m_server.healthy();
        }
    };
}
}