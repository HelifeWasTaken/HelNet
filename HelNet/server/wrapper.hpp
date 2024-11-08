/*
This code is licensed under the GNU GPL v3.
Copyright: (C) 2024 Mattis DALLEAU
*/

#pragma once

#include "HelNet/server/callbacks.hpp"
#include "HelNet/server/plugins.hpp"

namespace hl
{
namespace net
{
    template<class Protocol>
    class server_wrapper final : public hl::silva::collections::meta::NonCopyMoveable
    {
    private:
        server_t m_shared_server;
        Protocol &m_server;

        plugins::plugin_manager<plugins::server_plugin> m_plugins;

    public:
        server_wrapper()
            : m_shared_server(boost::static_pointer_cast<base_abstract_server_unwrapped>(Protocol::make()))
            , m_server(*boost::dynamic_pointer_cast<Protocol>(this->m_shared_server))
            , m_plugins()
        {
            HL_NET_LOG_DEBUG("Creating server wrapper for server: {}", m_server.get_alias());

            m_server.set_alias(fmt::format("{}({})", typeid(Protocol).name(), reinterpret_cast<void*>(&m_server)));

HL_NET_DIAGNOSTIC_PUSH()
HL_NET_DIAGNOSTIC_UNUSED_PARAMETER_IGNORED()
            server_callbacks server_callbacks;
            server_callbacks.on_start_success_callback = HL_NET_SERVER_ON_START(server) { HL_NET_LOG_INFO("Server started: {}", server->get_alias()); };
            server_callbacks.on_stop_success_callback = HL_NET_SERVER_ON_STOP() { HL_NET_LOG_INFO("Server stopped: {}"); };
            server_callbacks.on_stop_error_callback = HL_NET_SERVER_ON_STOP_ERROR(ec) { HL_NET_LOG_ERROR("Server stop error: {}", ec.message()); };
            server_callbacks.on_connection_callback = HL_NET_SERVER_ON_CONNECTION(server, client) { HL_NET_LOG_INFO("Server accepted connection: {} - {}", server->get_alias(), client->get_alias()); };
            server_callbacks.on_connection_error_callback = HL_NET_SERVER_ON_CONNECTION_ERROR(server, ec) {
                HL_NET_LOG_ERROR("Server connection error: {} - {}", server ? server->get_alias() : "nullserver", ec.message());
            };
            server_callbacks.on_disconnection_callback = HL_NET_SERVER_ON_DISCONNECTION(server, client_id) { HL_NET_LOG_INFO("Server disconnected: {} - {}", server->get_alias(), client_id); };
            server_callbacks.on_disconnection_error_callback = HL_NET_SERVER_ON_DISCONNECTION_ERROR(server, ec) {
                HL_NET_LOG_ERROR("Server disconnection error: {} - {}", server ? server->get_alias() : "nullserver", ec.message());
            };
            server_callbacks.on_sent_callback = HL_NET_SERVER_ON_SENT(server, client, sent_bytes) { HL_NET_LOG_INFO("Server sent: {} - {} - {}", server->get_alias(), client->get_alias(), sent_bytes); };
            server_callbacks.on_send_error_callback = HL_NET_SERVER_ON_SEND_ERROR(server, client, ec, sent_bytes) {
                HL_NET_LOG_ERROR("Server send error: {} - {} - {} - {}", server ? server->get_alias() : "nullserver", client ? client->get_alias() : "nullclient", ec.message(), sent_bytes);
            };
            server_callbacks.on_receive_callback = HL_NET_SERVER_ON_RECEIVE(server, client, buffer_copy, recv_bytes) { HL_NET_LOG_INFO("Server received: {} - {} - {}", server->get_alias(), client->get_alias(), recv_bytes); };
            server_callbacks.on_receive_error_callback = HL_NET_SERVER_ON_RECEIVE_ERROR(server, client, buffer_copy, ec, recv_bytes) {
                HL_NET_LOG_ERROR("Server receive error: {} - {} - {} - {}", server ? server->get_alias() : "nullserver", client ? client->get_alias() : "nullclient", ec.message(), recv_bytes);
            };
HL_NET_DIAGNOSTIC_POP()

            m_server.callbacks_register().add_layer(DEFAULT_REGISTER_LAYER, server_callbacks);
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

        template<class Plugin, class... Args>
        void attach_plugin(Args&&... args)
        {
            return m_plugins.attach<Plugin>(this->m_shared_server, std::forward<Args>(args)...);
        }

        template<class Plugin>
        void detach_plugin()
        {
            m_plugins.detach<Plugin>(this->m_shared_server);
        }

        bool update()
        {
            this->m_plugins.update(this->m_shared_server);
            return this->healthy();
        }
    };
}
}
