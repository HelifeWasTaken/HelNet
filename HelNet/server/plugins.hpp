/*
This code is licensed under the GNU GPL v3.
Copyright: (C) 2024 Mattis DALLEAU
*/

#pragma once

#include "../base_plugins.hpp"
#include "abstract_server_unwrapped.hpp"

namespace hl
{
namespace net
{
namespace plugins
{
    using server_plugin = base_plugin<server_t, server_callbacks>;
    using server_plugin_manager = plugin_manager<server_plugin>;

    class server_clients_timeout : public server_plugin 
    {
    public:
        using time_point_t = std::chrono::time_point<std::chrono::high_resolution_clock>;

    private:
        std::unordered_map<client_id_t, time_point_t> client_timeouts;
        std::mutex mutex;
        const long int timeout;

    public:
        void on_connect(const client_id_t &client_id)
        {
            std::lock_guard<std::mutex> lock(mutex);
            client_timeouts.emplace(client_id, std::chrono::high_resolution_clock::now());
            HL_NET_LOG_DEBUG("server_clients_timeout: Client connected: {}", client_id);
        }

        void on_disconnect(const client_id_t &client_id)
        {
            std::lock_guard<std::mutex> lock(mutex);
            client_timeouts.erase(client_id);
            HL_NET_LOG_DEBUG("server_clients_timeout: Client disconnected: {}", client_id);
        }

        void on_receive(const client_id_t &client_id)
        {
            std::lock_guard<std::mutex> lock(mutex);
            auto it = client_timeouts.find(client_id);
            HL_NET_LOG_DEBUG("server_clients_timeout: On Receive: {}", client_id);
            if (it != client_timeouts.end())
            {
                it->second = std::chrono::high_resolution_clock::now();
            }
            else
            {
                on_connect(client_id);
            }
        }

    public:
        server_clients_timeout(const long int &ms)
            : client_timeouts()
            , mutex()
            , timeout(ms)
        {}

        bool require_connection_on() const override final
        {
            return true;
        }

        size_t on_update(server_t &server) override final
        {
            std::lock_guard<std::mutex> lock(mutex);
            const auto now = std::chrono::high_resolution_clock::now();
            long int smalldiff = 0;
            for (auto it = client_timeouts.begin(); it != client_timeouts.end();)
            {
                const long int diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - it->second).count();
                if (diff > timeout)
                {
                    HL_NET_LOG_DEBUG("server_clients_timeout: Client timeout: {}", it->first);
                    server->disconnect(it->first);
                    it = client_timeouts.erase(it);
                }
                else
                {
                    HL_NET_LOG_DEBUG("server_clients_timeout: Client {} will timeout in: {} ms", it->first, timeout - diff);
                    ++it;
                    smalldiff = std::min(smalldiff, diff);
                }
            }
            return std::max(smalldiff, 0L);
        }

        server_callbacks callbacks() override final
        {
            server_callbacks callbacks;
            callbacks.on_connection_callback    = [this](server_t, connection_t client) { this->on_connect(client->get_id()); };
            callbacks.on_disconnection_callback = [this](server_t, const client_id_t& id) { this->on_disconnect(id); };
            callbacks.on_receive_callback       = [this](server_t, connection_t client, shared_buffer_t, const size_t) { this->on_receive(client->get_id()); };
            return callbacks;
        }
    };

}
}
}