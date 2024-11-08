/*
This code is licensed under the GNU GPL v3.
Copyright: (C) 2024 Mattis DALLEAU
*/

#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/smart_ptr.hpp>
#include "HelNet/server/abstract_connection_unwrapped.hpp"

namespace hl
{
namespace net
{
    class tcp_connection_unwrapped final : public base_abstract_connection_unwrapped
    {
    public:
        using shared_t = boost::shared_ptr<tcp_connection_unwrapped>;

    private:
        boost::asio::ip::tcp::socket m_socket;
        std::mutex m_mutex_api_control_flow;

        void _receive_async_callback(const boost::system::error_code &ec,
                                    size_t bytes_transferred,
                                    connection_t connection,
                                    shared_buffer_t receive_buffer)
        {
            shared_buffer_t buffer_cpy = make_shared_buffer(receive_buffer, bytes_transferred);

            HL_NET_LOG_DEBUG("Received {} bytes from connection: {}", bytes_transferred, get_alias());
            if (ec)
            {
                HL_NET_LOG_WARN("Error on receive for connection from: {} with error: {}", get_alias(), ec.message());
                switch (ec.value())
                {
                _HL_INTERNAL_UNHEALTHY_CASES_CONNECTION_UNHEALTHY:
                    HL_NET_LOG_ERROR("Connection cannot receive data from: {} due to {}, stopping receive, connection is not healthy!", get_alias(), ec.message());
                    this->set_health_status(false);
                    notify_client_as_unhealthy_to_the_server();
                    break;

                _HL_INTERNAL_UNHEALTHY_CASES_SERVER_FROM_CONNECTION:
                    HL_NET_LOG_ERROR("Connection cannot receive data from: {} due to {}, stopping receive, server is not healthy!", get_alias(), ec.message());
                    notify_server_as_unhealthy();
                    break;

                default:
                    break;
                }
                this->callbacks_register().on_receive_error(connection, buffer_cpy, ec, bytes_transferred);
            }
            else
            {
                this->callbacks_register().on_receive(connection, buffer_cpy, bytes_transferred);
            }

            _receive_async();
        }


        void _receive_async()
        {
            std::lock_guard<std::mutex> lock(m_mutex_api_control_flow);

            connection_t connection = shared_from_this();
            shared_buffer_t buffer = receive_buffer();

            if (!healthy())
            {
                HL_NET_LOG_ERROR("Cannot read: Connection is not healthy: {} may be either disconnected or received a non-recoverable error", get_alias());
                callbacks_register().on_receive_error(connection, buffer, boost::asio::error::not_connected, 0);
                return;
            }

            HL_NET_LOG_DEBUG("Start reading for connection: {}", get_id());

            m_socket.async_receive(
                boost::asio::buffer(*buffer),
                [this, connection, buffer]
                (const boost::system::error_code &ec, const size_t &bytes_transferred) {
                    _receive_async_callback(ec, bytes_transferred, connection, buffer);
                }
            );
        }

    private:
        tcp_connection_unwrapped(boost::asio::io_service &io_service,
                                server_callback_register &callback_register,
                                const server_is_unhealthy_notifier_t& notify_server_as_unhealthy,
                                const client_is_unhealthy_notifier_t& notify_client_as_unhealthy_to_the_server)
            : base_abstract_connection_unwrapped(callback_register, notify_server_as_unhealthy, notify_client_as_unhealthy_to_the_server)
            , m_socket(io_service)
            , m_mutex_api_control_flow()
        {
            HL_NET_LOG_TRACE("Creating connection_t: {}", get_alias());
            set_run_status(true);
            set_health_status(true);
        }

    public:
        static shared_t make(boost::asio::io_service &io_service,
                              server_callback_register &callback_register,
                              const std::function<void(void)>& notify_server_as_unhealthy,
                              const std::function<void(const client_id_t&)>& notify_client_as_unhealthy_to_the_server)
        {
            return shared_t(new tcp_connection_unwrapped(io_service, callback_register, notify_server_as_unhealthy, notify_client_as_unhealthy_to_the_server));
        }

        virtual ~tcp_connection_unwrapped() override final
        {
            HL_NET_LOG_TRACE("Destroying connection_t: {}", get_alias());
            if (is_running())
            {
                stop();
            }
            HL_NET_LOG_TRACE("Destroyed connection_t: {}", get_alias());
        }

        boost::asio::ip::tcp::socket &socket()
        {
            return m_socket;
        }

        bool stop() override final
        {
            std::lock_guard<std::mutex> lock(m_mutex_api_control_flow);

            if (!is_running())
            {
                HL_NET_LOG_WARN("Connection already stopped: {}", get_alias());
                return false;
            }
            HL_NET_LOG_TRACE("Stopping connection: {}", get_alias());
            set_run_status(false);
            set_health_status(false);
            m_socket.close();
            HL_NET_LOG_TRACE("Stopped connection: {}", get_alias());
            return true;
        }
    
    private:
        void _send_async_callback(const boost::system::error_code &ec, const size_t bytes_transferred, connection_t connexion)
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
        bool send(const shared_buffer_t &buffer, const size_t &size) override final
        {
            std::lock_guard<std::mutex> lock(m_mutex_api_control_flow);
            connection_t connexion = shared_from_this();

            HL_NET_LOG_DEBUG("Preparing sending {} bytes to: {}", size, get_alias());

            if (!healthy())
            {
                HL_NET_LOG_ERROR("Cannot send data to a non-healthy connection: {}", get_alias());
                callbacks_register().on_send_error(connexion, boost::asio::error::not_connected, 0);
                return false;
            }
            else if (!size)
            {
                HL_NET_LOG_ERROR("Cannot send 0 bytes to: {}", get_alias());
                callbacks_register().on_send_error(connexion, boost::asio::error::invalid_argument, 0);
                return false;
            }
            else if (!buffer)
            {
                HL_NET_LOG_ERROR("Cannot send data from a null buffer to: {}", get_alias());
                callbacks_register().on_send_error(connexion, boost::asio::error::invalid_argument, 0);
                return false;
            }
            else if (size > buffer->size())
            {
                HL_NET_LOG_ERROR("Cannot send more than the buffer size: {} bytes to connection: {}", buffer->size(), get_alias());
                callbacks_register().on_send_error(connexion, boost::asio::error::message_size, buffer->size());
                return false;
            }
            else
            {
                HL_NET_LOG_DEBUG("Sending {} bytes to connection: {}", size, get_alias());

                m_socket.async_send(
                    boost::asio::buffer(*buffer, size),
                    [this, connexion](const boost::system::error_code &ec, const size_t bytes_transferred) {
                        _send_async_callback(ec, bytes_transferred, connexion);
                    }
                );
                return true;
            }
        }

        void start_receive()
        {
            _receive_async();
        }
    };
}
}
