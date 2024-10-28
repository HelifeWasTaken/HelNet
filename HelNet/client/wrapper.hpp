/*
This code is licensed under the GNU GPL v3.
Copyright: (C) 2024 Mattis DALLEAU
*/

#pragma once

#include <hl/silva/collections/meta.hpp>

namespace hl
{
namespace net
{
    template<class Protocol>
    class client_wrapper : public hl::silva::collections::meta::NonCopyMoveable
    {
    private:
        typename Protocol::shared_t m_shared_client;
        Protocol &m_client;

    public:
        explicit client_wrapper()
            : m_shared_client(Protocol::make())
            , m_client(*this->m_shared_client)
        {
            HL_NET_LOG_TRACE("Creating client wrapper for: {}", this->get_alias());

            client_callback_register &callbacks = this->m_client.callbacks_register();

#if __GNUC__ || __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
            callbacks.set_on_connect(HL_NET_CLIENT_ON_CONNECT(client) { HL_NET_LOG_INFO("Client connected: {}", client->get_alias()); });
            callbacks.set_on_disconnect(HL_NET_CLIENT_ON_DISCONNECT() { HL_NET_LOG_INFO("Client disconnected..."); });
            callbacks.set_on_disconnect_error(HL_NET_CLIENT_ON_DISCONNECT_ERROR(ec) { HL_NET_LOG_ERROR("Client disconnect error: {}", ec.message()); });
            callbacks.set_on_receive(HL_NET_CLIENT_ON_RECEIVE(client, buffer_copy, recv_bytes) { HL_NET_LOG_INFO("Client received: {} - {}", client->get_alias(), recv_bytes); });
            callbacks.set_on_receive_error(HL_NET_CLIENT_ON_RECEIVE_ERROR(client, buffer_copy, ec, recv_bytes) { HL_NET_LOG_ERROR("Client receive error: {} - {} - {}", client->get_alias(), ec.message(), recv_bytes); });
            callbacks.set_on_sent(HL_NET_CLIENT_ON_SENT(client, sent_bytes) { HL_NET_LOG_INFO("Client sent: {} - {}", client->get_alias(), sent_bytes); });
            callbacks.set_on_send_error(HL_NET_CLIENT_ON_SEND_ERROR(client, ec, sent_bytes) { HL_NET_LOG_ERROR("Client send error: {} - {} - {}", client->get_alias(), ec.message(), sent_bytes); });

#if __GNUC__ || __clang__
#pragma GCC diagnostic pop
#endif

            HL_NET_LOG_TRACE("Created client wrapper for client: {}", this->get_alias());
        }

        ~client_wrapper()
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

        operator bool() const
        {
            return this->healthy();
        }
    };
}
}