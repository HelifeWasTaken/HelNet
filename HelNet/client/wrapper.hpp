/*
This code is licensed under the GNU GPL v3.
Copyright: (C) 2024 Mattis DALLEAU
*/

#pragma once

#include <hl/silva/collections/meta.hpp>
#include "HelNet/client/plugins.hpp"

namespace hl
{
namespace net
{
    template<class Protocol>
    class client_wrapper final : public hl::silva::collections::meta::NonCopyMoveable
    {
    private:
        typename Protocol::shared_t m_shared_client;
        Protocol &m_client;

        plugins::plugin_manager<plugins::client_plugin> m_plugins;

    public:
        explicit client_wrapper()
            : m_shared_client(Protocol::make())
            , m_client(*this->m_shared_client)
            , m_plugins()
        {
            HL_NET_LOG_TRACE("Creating client wrapper for: {}", this->get_alias());

HL_NET_DIAGNOSTIC_PUSH()
HL_NET_DIAGNOSTIC_UNUSED_PARAMETER_IGNORED()
            client_callbacks client_callbacks;
            client_callbacks.on_connect_callback = HL_NET_CLIENT_ON_CONNECT(client) { HL_NET_LOG_INFO("Client connected: {}", client->get_alias()); };  
            client_callbacks.on_disconnect_callback = HL_NET_CLIENT_ON_DISCONNECT() { HL_NET_LOG_INFO("Client disconnected..."); };
            client_callbacks.on_disconnect_error_callback = HL_NET_CLIENT_ON_DISCONNECT_ERROR(ec) {
                HL_NET_LOG_ERROR("Client disconnect error: {}", ec.message());
            };
            client_callbacks.on_receive_callback = HL_NET_CLIENT_ON_RECEIVE(client, buffer_copy, recv_bytes) { HL_NET_LOG_INFO("Client received: {} - {}", client->get_alias(), recv_bytes); };
            client_callbacks.on_receive_error_callback = HL_NET_CLIENT_ON_RECEIVE_ERROR(client, buffer_copy, ec, recv_bytes) {
                HL_NET_LOG_ERROR("Client receive error: {} - {} - {}", client ? client->get_alias() : "nullclient", ec.message(), recv_bytes);
            };
            client_callbacks.on_sent_callback = HL_NET_CLIENT_ON_SENT(client, sent_bytes) { HL_NET_LOG_INFO("Client sent: {} - {}", client->get_alias(), sent_bytes); };
            client_callbacks.on_send_error_callback = HL_NET_CLIENT_ON_SEND_ERROR(client, ec, sent_bytes) {
                HL_NET_LOG_ERROR("Client send error: {} - {} - {}", client ? client->get_alias() : "nullclient", ec.message(), sent_bytes);
            };
HL_NET_DIAGNOSTIC_POP()

            m_client.callbacks_register().add_layer(DEFAULT_REGISTER_LAYER, client_callbacks);
            HL_NET_LOG_TRACE("Created client wrapper for client: {}", this->get_alias());
        }

        virtual ~client_wrapper() override final
        {
            HL_NET_LOG_TRACE("Destroying client wrapper for client: {}", this->get_alias());
            HL_NET_LOG_TRACE("Destroyed client wrapper for client: {}", this->get_alias());
        }

        // API Reproduction

        const std::string get_alias() const
        {
            return this->m_client.get_alias();
        }

        void set_alias(const std::string &alias)
        {
            this->m_client.set_alias(alias);
        }

        client_callback_register& callbacks_register()
        {
            return this->m_client.callbacks_register();
        }

        bool connected() const
        {
            return this->m_client.connected();
        }

        bool healthy() const
        {
            return this->m_client.healthy();
        }

        bool connect(const std::string &host, const std::string &port, bool auto_alias = true)
        {
            if (auto_alias) {
                this->set_alias(fmt::format("client({}, {}:{})", static_cast<void*>(this), host, port));
            }
            return this->m_client.connect(host, port);
        }

        bool disconnect(void)
        {
            return this->m_client.disconnect();
        }

        bool send(const shared_buffer_t &buffer)
        {
            return this->m_client.send(buffer);
        }

        bool send(const shared_buffer_t &buffer, const size_t &size)
        {
            return this->m_client.send(buffer, size);
        }

        bool send_bytes(const void *data, const size_t &size)
        {
            return this->m_client.send_bytes(data, size);
        }

        template<typename T>
        bool send_bytes(const std::vector<T> &data)
        {
            return this->m_client.send_bytes(data);
        }

        bool send_string(const std::string &str)
        {
            return this->m_client.send_string(str);
        }

        template<class Plugin, class... Args>
        void attach_plugin(Args... args)
        {
            this->m_plugins.attach<Plugin>(this->m_shared_client, args...);
        }

        template<class Plugin>
        void detach_plugin()
        {
            this->m_plugins.detach<Plugin>(this->m_shared_client);
        }

        bool update()
        {
            // this->m_plugins.update(this->m_shared_client);
            return this->healthy();
        }

    };
}
}