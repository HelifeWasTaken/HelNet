/*
This code is licensed under the GNU GPL v3.
Copyright: (C) 2024 Mattis DALLEAU
*/

#pragma once

#include <boost/asio/ip/udp.hpp>

#include "HelNet/server/abstract_connection_unwrapped.hpp"
#include "HelNet/server/utils.hpp"

namespace hl
{
namespace net
{
    class udp_connection_unwrapped final : public base_abstract_connection_unwrapped
    {
    public:
        using shared_t = std::shared_ptr<udp_connection_unwrapped>;
    
    private:
        boost::asio::ip::udp::socket &m_socket;
        const boost::asio::ip::udp::endpoint m_endpoint;
        const std::string m_endpoint_str;

        std::mutex m_mutex_api_control_flow;

        udp_connection_unwrapped(server_callback_register &callback_register,
                                const server_is_unhealthy_notifier_t& notify_server_as_unhealthy,
                                const client_is_unhealthy_notifier_t& notify_client_as_unhealthy_to_the_server,
                                const boost::asio::ip::udp::endpoint &endpoint,
                                boost::asio::ip::udp::socket &socket)
            : base_abstract_connection_unwrapped(callback_register, notify_server_as_unhealthy, notify_client_as_unhealthy_to_the_server)
            , m_socket(socket)
            , m_endpoint(endpoint)
            , m_endpoint_str(utils::endpoint_to_string(endpoint))
            , m_mutex_api_control_flow()
        {
            HL_NET_LOG_DEBUG("Creating udp_connection_unwrapped: {}", get_alias());
            set_run_status(true);
            set_health_status(true);

            set_alias(fmt::format("udp_connection_unwrapped({})", m_endpoint_str));
        }

    public:
        static shared_t make(server_callback_register &callback_register,
                            const server_is_unhealthy_notifier_t& notify_server_as_unhealthy,
                            const client_is_unhealthy_notifier_t& notify_client_as_unhealthy_to_the_server,
                            const boost::asio::ip::udp::endpoint &endpoint,
                            boost::asio::ip::udp::socket &socket)
        {
            return shared_t(new udp_connection_unwrapped(callback_register, notify_server_as_unhealthy, notify_client_as_unhealthy_to_the_server, endpoint, socket));
        }

        const boost::asio::ip::udp::endpoint &endpoint()
        {
            return m_endpoint;
        }

        virtual ~udp_connection_unwrapped() override final
        {
            HL_NET_LOG_TRACE("Destroying udp_connection_unwrapped: {}", get_alias());
            if (is_running())
            {
                stop();
            }
            HL_NET_LOG_TRACE("Destroyed udp_connection_unwrapped: {}", get_alias());
        }

        bool stop() override final
        {
            std::lock_guard<std::mutex> lock(m_mutex_api_control_flow);

            HL_NET_LOG_DEBUG("Stopping connection: {}", get_alias());
            if (!is_running())
            {
                HL_NET_LOG_WARN("Connection already stopped: {}", get_alias());
                callbacks_register().on_stop_error(boost::system::error_code(boost::asio::error::not_connected));
                return false;
            }
            set_run_status(false);
            set_health_status(false);
            HL_NET_LOG_DEBUG("Stopped connection: {}", get_alias());
            return true;
        }

        const std::string& get_endpoint_id() const
        {
            return m_endpoint_str;
        }
    
    private:
        void _send_async_connexion_callback(const boost::system::error_code &ec, const size_t bytes_transferred, connection_t connexion)
        {
            HL_NET_LOG_DEBUG("Sent {} bytes to connection: {}", bytes_transferred, get_alias());
            if (ec)
            {
                HL_NET_LOG_WARN("Error on send to connection: {} with error: {}", get_alias(), ec.message());
                this->callbacks_register().on_send_error(connexion, ec, bytes_transferred);
                switch (ec.value())
                {
                _HL_INTERNAL_UNHEALTHY_CASES_CONNECTION_UNHEALTHY:
                    HL_NET_LOG_ERROR("Connection cannot send data to: {} due to {}, stopping receive, connection is not healthy!", get_alias(), ec.message());
                    this->set_health_status(false);
                    notify_client_as_unhealthy_to_the_server();
                    break;
                _HL_INTERNAL_UNHEALTHY_CASES_SERVER_FROM_CONNECTION:
                    HL_NET_LOG_ERROR("Connection cannot send data to: {} due to {}, stopping receive, server & connection is not healthy!", get_alias(), ec.message());
                    notify_server_as_unhealthy();
                    break;
                default:
                    break;
                }
            }
            else
            {
                this->callbacks_register().on_sent(connexion, bytes_transferred);
            }
        }

    public:
        bool send(const shared_buffer_t &buffer, const size_t &size) override
        {
            connection_t connexion = shared_from_this();

            if (!healthy())
            {
                HL_NET_LOG_ERROR("Cannot send data to a non-healthy connection: {}", get_alias());
                callbacks_register().on_send_error(connexion, boost::system::error_code(boost::asio::error::not_connected), 0);
                return false;
            }
            else if (!size)
            {
                HL_NET_LOG_ERROR("Cannot send 0 bytes to: {}", get_alias());
                callbacks_register().on_send_error(connexion, boost::system::error_code(boost::asio::error::invalid_argument), 0);
                return false;
            }
            else if (!buffer)
            {
                HL_NET_LOG_ERROR("Cannot send data from a null buffer to: {}", get_alias());
                callbacks_register().on_send_error(connexion, boost::system::error_code(boost::asio::error::invalid_argument), 0);
                return false;
            }
            else if (size > buffer->size())
            {
                HL_NET_LOG_ERROR("Cannot send more than the buffer size: {} bytes to connection: {}", buffer->size(), get_alias());
                callbacks_register().on_send_error(connexion, boost::system::error_code(boost::asio::error::invalid_argument), 0);
                return false;
            }

            HL_NET_LOG_DEBUG("Sending {} bytes to connection: {}", size, get_alias());
            m_socket.async_send_to(
                boost::asio::buffer(*buffer, size),
                m_endpoint,
                [this, buffer, connexion]
                (const boost::system::error_code &ec, const size_t bytes_transferred)
                {
                    _send_async_connexion_callback(ec, bytes_transferred, connexion);
                }
            );
            return true;
        }
    };
}
}