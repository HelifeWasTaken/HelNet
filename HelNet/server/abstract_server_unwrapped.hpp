/*
This code is licensed under the GNU GPL v3.
Copyright: (C) 2024 Mattis DALLEAU
*/

#pragma once

#include <boost/asio/io_service.hpp>

#include "HelNet/server/callbacks.hpp"
#include "HelNet/server/abstract_connection_unwrapped.hpp"
#include "HelNet/server/utils.hpp"

namespace hl
{
namespace net
{
    // Server
    using client_holder_t = std::unordered_map<client_id_t, connection_t>;
    using client_holder_name_to_id_t = utils::back_and_forth_unordered_map<std::string, client_id_t>;

HL_NET_DIAGNOSTIC_PUSH()
HL_NET_DIAGNOSTIC_NON_VIRTUAL_DESTRUCTOR_IGNORED()
    class base_abstract_server_unwrapped : public std::enable_shared_from_this<base_abstract_server_unwrapped>, public hl::silva::collections::meta::NonCopyMoveable
    {
HL_NET_DIAGNOSTIC_POP()
    public:
        using this_type_t = base_abstract_server_unwrapped;
        using shared_t = boost::shared_ptr<base_abstract_server_unwrapped>;

    private:
        server_callback_register m_callback_register;

        std::string m_alias;
        mutable std::mutex m_alias_mutex;

        std::atomic_bool m_running;
        std::atomic_bool m_healthy;

        boost::asio::io_service m_io_service;
        std::thread m_io_service_thread;

        atomic_client_id m_last_id;
        client_holder_t m_connections;
        client_holder_name_to_id_t m_connections_name_to_id;

        unhealthy_connexions_t m_unhealthy_connections;
        std::thread m_unhealthy_connections_thread;
        std::condition_variable m_unhealthy_connections_cv;
        mutable std::mutex m_unhealthy_connections_mutex;
        mutable std::mutex m_connections_mutex;

    protected:
        std::mutex m_mutex_api_control_flow;
    
    private:
        void basic_io_service_coroutine()
        {
            HL_NET_LOG_TRACE("Starting io_service coroutine for: {}", get_alias());
            while (healthy())
            {
                m_io_service.run();
                HL_NET_LOG_TRACE("Restarting io_service coroutine for: {}", get_alias());
                m_io_service.reset();
            }
            HL_NET_LOG_TRACE("Stopping io_service coroutine for: {}", get_alias());
        }

        void unhealthy_thread_check_coroutine()
        {
            HL_NET_LOG_TRACE("Starting unhealthy connections thread for server: {}", get_alias());
            while (true)
            {
                std::unique_lock<std::mutex> lock_waiter(m_unhealthy_connections_mutex);

                // wait that unealthy connections are available or that the server is not healthy
                m_unhealthy_connections_cv.wait(lock_waiter, [this]() -> bool {
                    return !m_unhealthy_connections.empty() || !this->healthy();
                });

                if (!this->healthy()) {
                    break;
                }
                client_id_t endpoint_id;
                {
                    std::lock_guard<std::mutex> lock_connections(m_connections_mutex);
                    while (!m_unhealthy_connections.empty()) {
                        endpoint_id = m_unhealthy_connections.front();
                        m_unhealthy_connections.pop();
                        if (this->healthy()) {
                            _unset_connection<false>(endpoint_id);
                        }
                    }
                }
            }
            HL_NET_LOG_TRACE("Stopping unhealthy connections thread for server (unhealthy or disconnected): {}", get_alias());
        }

    protected:
        void _unsafe_start()
        {
            HL_NET_LOG_TRACE("Starting server pool: {}", get_alias());

            callbacks_register().unsafe_start_pool();

            m_last_id = BASE_CLIENT_ID;

            set_run_status(true);
            set_health_status(true);

            m_io_service_thread = std::thread(
                std::bind(&this_type_t::basic_io_service_coroutine, this)
            );
            m_unhealthy_connections_thread = std::thread(
                std::bind(&this_type_t::unhealthy_thread_check_coroutine, this)
            );

            HL_NET_LOG_TRACE("Started server pool: {}", get_alias());
        }

        void _unsafe_stop()
        {
            HL_NET_LOG_TRACE("Stopping server pool: {}", get_alias());

            set_run_status(false);
            set_health_status(false);
            m_io_service.stop();
            m_io_service_thread.join();
            m_unhealthy_connections_cv.notify_one();
            m_unhealthy_connections_thread.join();
            callbacks_register().on_stop_success();
            callbacks_register().unsafe_stop_pool();
            {
                std::lock_guard<std::mutex> lock(m_connections_mutex);
                m_connections.clear();
            }
            HL_NET_LOG_TRACE("Stopped server pool: {}", get_alias());
        }

        template<bool LockConnectionMutex>
        connection_t _get_connection(const client_id_t& client_id)
        {
            _HL_INTERNAL_LOCK_GUARD_WHEN_TRUE(LockConnectionMutex, lock_conn, m_connections_mutex, {
                if (client_id == INVALID_CLIENT_ID) return nullptr;
                auto it = m_connections.find(client_id);
                return it == m_connections.end() ? nullptr : it->second;
            });
        }

        template<bool LockConnectionMutex>
        const connection_t _get_connection(const client_id_t& client_id) const
        {
            _HL_INTERNAL_LOCK_GUARD_WHEN_TRUE(LockConnectionMutex, lock_conn, m_connections_mutex, {
                if (client_id == INVALID_CLIENT_ID) return nullptr;
                auto it = m_connections.find(client_id);
                return it == m_connections.end() ? nullptr : it->second;
            });
        }

        template<bool LockConnectionMutex>
        bool _has_connection(const client_id_t& client_id) const
        {
            _HL_INTERNAL_LOCK_GUARD_WHEN_TRUE(LockConnectionMutex, lock_conn, m_connections_mutex, {
                return m_connections.find(client_id) != m_connections.end();
            });
        }

        template<bool LockConnectionMutex>
        connection_t _get_connection(const std::string &endpoint_id)
        {
            _HL_INTERNAL_LOCK_GUARD_WHEN_TRUE(LockConnectionMutex, lock_conn, m_connections_mutex, {
                client_holder_name_to_id_t::forward_iterator it = m_connections_name_to_id.find_forward(endpoint_id);
                return it == m_connections_name_to_id.end_forward() ? nullptr : _get_connection<false>(it->second);
            });
        }

        template<bool LockConnectionMutex>
        const connection_t _get_connection(const std::string &endpoint_id) const
        {
            _HL_INTERNAL_LOCK_GUARD_WHEN_TRUE(LockConnectionMutex, lock_conn, m_connections_mutex, {
                client_holder_name_to_id_t::const_forward_iterator it = m_connections_name_to_id.find_forward(endpoint_id);
                return it == m_connections_name_to_id.end_forward() ? nullptr : _get_connection<false>(it->second);
            });
        }

        template<bool LockConnectionMutex>
        bool _has_connection(const std::string &endpoint_id) const
        {
            _HL_INTERNAL_LOCK_GUARD_WHEN_TRUE(LockConnectionMutex, lock_conn, m_connections_mutex, {
                return m_connections_name_to_id.find_forward(endpoint_id) != m_connections_name_to_id.end_forward();
            });
        }

        template<bool LockConnectionMutex>
        void _set_connection(const connection_t& connection,
                             const std::string& name)
        {
            _HL_INTERNAL_LOCK_GUARD_WHEN_TRUE(LockConnectionMutex, lock, m_connections_mutex, {
                do {
                    for (; m_connections.find(m_last_id) != m_connections.end(); ++m_last_id);
                } while (m_last_id == INVALID_CLIENT_ID); // find a valid id in case of overflow
                connection->set_alias(name);
                connection->set_id(m_last_id++);
                m_connections.emplace(connection->get_id(), connection);
                m_connections_name_to_id.insert(name, connection->get_id());
            });
        }

        template<bool LockConnectionMutex>
        bool _unset_connection(const client_id_t& client_id)
        {
            _HL_INTERNAL_LOCK_GUARD_WHEN_TRUE(LockConnectionMutex, lock, m_connections_mutex, {
                HL_NET_LOG_ERROR("Unsetting connection: {} from server: {}", client_id, get_alias());
                client_holder_t::iterator it = m_connections.find(client_id);
                if (it == m_connections.end())
                {
                    HL_NET_LOG_ERROR("Cannot unset a non-existing connection: {} from server: {}", client_id, get_alias());
                    callbacks_register().on_disconnection_error(boost::asio::error::not_found);
                    return false;
                }
                m_connections.erase(it);
                m_connections_name_to_id.erase(client_id);
                return true;
            });
        }

        template<bool LockConnectionMutex>
        bool _unset_connection(const std::string &endpoint_id)
        {
            _HL_INTERNAL_LOCK_GUARD_WHEN_TRUE(LockConnectionMutex, lock, m_connections_mutex, {
                client_holder_name_to_id_t::forward_iterator it = m_connections_name_to_id.find_forward(endpoint_id);
                if (it == m_connections_name_to_id.end_forward())
                {
                    HL_NET_LOG_ERROR("Cannot unset a non-existing connection: {} from server: {}", endpoint_id, get_alias());
                    callbacks_register().on_disconnection_error(boost::asio::error::not_found);
                    return false;
                }
                m_connections.erase(it->second);
                m_connections_name_to_id.erase(it);
                return true;
            });
        }

        // utility function to lock the connections mutex and apply a function
        // required when get&set connections are done manually
        template<typename ReturnType>
        ReturnType _lock_connection_and_apply(const std::function<ReturnType()> &apply)
        {
            std::lock_guard<std::mutex> lock(m_connections_mutex);
            return apply();
        }

        boost::asio::io_service& _io_service()
        {
            return m_io_service;
        }

    public:
        client_is_unhealthy_notifier_t make_client_is_unhealthy_notifier()
        {
            return [this](const client_id_t& client_id) -> void {
                std::lock_guard<std::mutex> lock(m_unhealthy_connections_mutex);
                m_unhealthy_connections.push(client_id);
                m_unhealthy_connections_cv.notify_one();
            };
        }

        server_is_unhealthy_notifier_t make_server_is_unhealthy_notifier()
        {
            return [this](void) -> void { set_health_status(false); };
        }

        bool is_running() const
        {
            return m_running;
        }

        void set_run_status(const bool status)
        {
            HL_NET_LOG_WARN("Server: {} is now {}", get_alias(), status ? "running" : "stopped");
            m_running = status;
        }

        bool healthy() const
        {
            return m_healthy && is_running();
        }

        void set_health_status(const bool status)
        {
            HL_NET_LOG_WARN("Server: {} health is now {}", get_alias(), status ? "healthy" : "unhealthy");
            m_healthy = status;
        }

        // alias to set_health_status(false) to stop the server
        void request_stop(void)
        {
            set_health_status(false); // Mark as unhealthy
        }

        server_callback_register& callbacks_register()
        {
            return m_callback_register;
        }

        const std::string get_alias() const
        {
            std::lock_guard<std::mutex> lock(m_alias_mutex);
            return m_alias;
        }

        void set_alias(const std::string &alias)
        {
            HL_NET_LOG_INFO("Set alias for server: {} to: {}", get_alias(), alias);
            std::lock_guard<std::mutex> lock(m_alias_mutex);
            m_alias = alias;
        }

        server_t as_sharable()
        {
            return shared_from_this();
        }

    public:
        virtual bool start(const std::string &port) = 0;
        virtual bool stop() = 0;

    public:
        bool send(const client_id_t& client_id, const shared_buffer_t &buffer, const size_t &size)
        {
            std::lock_guard<std::mutex> lock(m_mutex_api_control_flow);
            HL_NET_LOG_DEBUG("Sending {} bytes to client: {} from server: {}", size, client_id, get_alias());

            connection_t connection = _get_connection<true>(client_id);
            if (!connection)
            {
                HL_NET_LOG_ERROR("Cannot send data to a non-existing connection: {} from server: {}", client_id, get_alias());
                connection_t conn_null = nullptr;
                callbacks_register().on_send_error(conn_null, boost::asio::error::not_connected, 0);
                return false;
            }
            return connection->send(buffer, size);
        }

        bool send(const std::string &endpoint_id, const shared_buffer_t &buffer, const size_t &size)
        {
            std::lock_guard<std::mutex> lock(m_mutex_api_control_flow);
            HL_NET_LOG_DEBUG("Sending {} bytes to client: {} from server: {}", size, endpoint_id, get_alias());

            connection_t connection = _get_connection<true>(endpoint_id);
            if (!connection)
            {
                HL_NET_LOG_ERROR("Cannot send data to a non-existing connection: {} from server: {}", endpoint_id, get_alias());
                connection_t null_connection = nullptr;
                callbacks_register().on_send_error(null_connection, boost::system::error_code(boost::asio::error::not_found), size);
                return false;
            }
            return connection->send(buffer, size);
        }

        bool disconnect(const client_id_t& client_id)
        {
            return _unset_connection<true>(client_id);
        }

        bool disconnect(const std::string &endpoint_id)
        {
            return _unset_connection<true>(endpoint_id);
        }

    protected:
        base_abstract_server_unwrapped()
            : m_callback_register([this]() -> server_t { return as_sharable(); })
            , m_alias(fmt::format("base_abstract_server_unwrapped({})", static_cast<void*>(this)))
            , m_alias_mutex()
            , m_running(false)
            , m_healthy(false)
            , m_io_service()
            , m_io_service_thread()
            , m_last_id()
            , m_connections()
            , m_connections_name_to_id()
            , m_unhealthy_connections()
            , m_unhealthy_connections_thread()
            , m_unhealthy_connections_cv()
            , m_unhealthy_connections_mutex()
            , m_connections_mutex()
            , m_mutex_api_control_flow()
        {
            HL_NET_LOG_TRACE("Creating base_abstract_server_unwrapped: {}", get_alias());
        }

    public:
        virtual ~base_abstract_server_unwrapped() override
        {
            HL_NET_LOG_TRACE("Destroying base_abstract_server_unwrapped: {}", get_alias());
        }

    public:
        bool send(const client_id_t& client_id, const shared_buffer_t &buffer)
        {
            return send(client_id, buffer, buffer->size());
        }

        bool send_bytes(const client_id_t& client_id, const byte *data, const size_t &size)
        {
            shared_buffer_t buffer = make_shared_buffer(data, size);
            return send(client_id, buffer, size);
        }

        template<typename T>
        inline bool send_bytes(const client_id_t& client_id, const std::vector<T> &data)
        {
            return send_bytes(client_id, reinterpret_cast<const byte *>(data.data()), data.size() * sizeof(T));
        }

        inline bool send_string(const client_id_t& client_id, const std::string &str)
        {
            return send_bytes(client_id, reinterpret_cast<const byte *>(str.data()), str.size());
        }
    };

}
}
