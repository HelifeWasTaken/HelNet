/*
This code is licensed under the GNU GPL v3.
Copyright: (C) 2024 Mattis DALLEAU
*/

#pragma once

#include "HelNet/server/callbacks.hpp"

namespace hl
{
namespace net
{
    using client_is_unhealthy_notifier_t = std::function<void(const client_id_t&)>;
    using server_is_unhealthy_notifier_t = std::function<void(void)>;

HL_NET_DIAGNOSTIC_PUSH()
HL_NET_DIAGNOSTIC_NON_VIRTUAL_DESTRUCTOR_IGNORED()
    class base_abstract_connection_unwrapped : public boost::enable_shared_from_this<base_abstract_connection_unwrapped>, public hl::silva::collections::meta::NonCopyMoveable
    {
HL_NET_DIAGNOSTIC_POP()
    public:
        using shared_t = boost::shared_ptr<base_abstract_connection_unwrapped>;

        virtual bool stop() = 0;
        virtual bool send(const shared_buffer_t &buffer, const size_t &size) = 0;

    private:
        server_callback_register &m_callback_register;
        shared_buffer_t m_receive_buffer;

        std::string m_alias;
        mutable std::mutex m_alias_mutex;

        atomic_client_id m_id;
        std::atomic_bool m_healthy;
        std::atomic_bool m_running;

        const server_is_unhealthy_notifier_t m_notify_server_as_unhealthy;
        const client_is_unhealthy_notifier_t m_notify_client_as_unhealthy_to_the_server;

    protected:
        shared_buffer_t receive_buffer()
        {
            return m_receive_buffer;
        }

        void set_run_status(const bool status)
        {
            HL_NET_LOG_INFO("Set run status for connection: {} to: {}", get_alias(), status ? "running" : "stopped");
            m_running = status;
        }

        void set_health_status(const bool status)
        {
            HL_NET_LOG_INFO("Set health status for connection: {} to: {}", get_alias(), status ? "healthy" : "unhealthy");
            m_healthy = status;
        }

        void notify_server_as_unhealthy()
        {
            m_notify_server_as_unhealthy();
        }

        void notify_client_as_unhealthy_to_the_server()
        {
            m_notify_client_as_unhealthy_to_the_server(get_id());
        }

    public:
        server_callback_register& callbacks_register()
        {
            return m_callback_register;
        }

        std::string get_alias() const
        {
            std::lock_guard<std::mutex> lock(m_alias_mutex);
            return m_alias;
        }

        void set_alias(const std::string &alias)
        {
            HL_NET_LOG_INFO("Set alias for connection: {} to: {}", get_alias(), alias);
            std::lock_guard<std::mutex> lock(m_alias_mutex);
            m_alias = alias;
        }

        void set_id(const client_id_t &id)
        {
            HL_NET_LOG_INFO("Set id for connection: {} to: {}", id, get_alias());
            m_id = id;
        }

        client_id_t get_id() const
        {
            return m_id;
        }

        inline bool connected() const
        {
            return get_id() != INVALID_CLIENT_ID;
        }

        inline bool is_running() const
        {
            return m_running && connected();
        }

        inline bool healthy() const
        {
            return m_healthy && is_running();
        }

    protected:
        base_abstract_connection_unwrapped(server_callback_register &callback_register,
                                            const server_is_unhealthy_notifier_t& notify_server_as_unhealthy,
                                            const client_is_unhealthy_notifier_t& notify_client_as_unhealthy_to_the_server)
            : m_callback_register(callback_register)
            , m_receive_buffer(make_shared_buffer())
            , m_alias(fmt::format("base_abstract_connection_unwrapped({})", static_cast<void*>(this)))
            , m_alias_mutex()
            , m_id(INVALID_CLIENT_ID)
            , m_healthy(false)
            , m_running(false)
            , m_notify_server_as_unhealthy(notify_server_as_unhealthy)
            , m_notify_client_as_unhealthy_to_the_server(notify_client_as_unhealthy_to_the_server)
        {
            HL_NET_LOG_TRACE("Creating base_abstract_connection_unwrapped: {}", get_alias());
        }

    public:
        virtual ~base_abstract_connection_unwrapped() override
        {
            HL_NET_LOG_TRACE("Destroying base_abstract_connection_unwrapped: {}", get_alias());
            HL_NET_LOG_TRACE("Destroyed base_abstract_connection_unwrapped: {}", get_alias());
        }
    };

    using connection_t = base_abstract_connection_unwrapped::shared_t;
}
}
