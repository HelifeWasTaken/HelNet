/*
This code is licensed under the GNU GPL v3.
Copyright: (C) 2024 Mattis DALLEAU
*/

#pragma once

#include "HelNet/base_plugins.hpp"
#include "HelNet/client/unwrapped.hpp"

namespace hl
{
namespace net
{
namespace plugins
{
    using client_plugin = base_plugin<client_t, client_callbacks>;

    class client_timeout final : public client_plugin
    {
    public:
        using time_point_t = std::chrono::time_point<std::chrono::high_resolution_clock>;
    
    private:
        time_point_t last_receive;
        const long int timeout;
    
    public:
        void on_receive()
        {
            last_receive = std::chrono::high_resolution_clock::now();
        }

    public:
        client_timeout(const long int &ms)
            : last_receive(std::chrono::high_resolution_clock::now())
            , timeout(ms)
        {}

        virtual ~client_timeout() override final = default;

        bool require_connection_on() const override final
        {
            return true;
        }

        void on_update(client_t &client) override final
        {
            const auto now = std::chrono::high_resolution_clock::now();
            const long int diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_receive).count();
            if (diff > timeout)
            {
                HL_NET_LOG_DEBUG("client_timeout: Client timeout: {}", client->get_alias());
                client->disconnect();
            }
            else
            {
                // HL_NET_LOG_DEBUG("client_timeout: Client {} will timeout in: {} ms", client->get_alias(), timeout - diff);
            }
        }

        client_callbacks callbacks() override final
        {
            client_callbacks callbacks;
            callbacks.on_connect_callback = [this](client_t) { this->on_receive(); };
            return callbacks;
        }
    };

}
}
}
