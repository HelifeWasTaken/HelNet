/*
This code is licensed under the GNU GPL v3.
Copyright: (C) 2024 Mattis DALLEAU
*/

#pragma once

#include <boost/asio/ip/udp.hpp>
#include <boost/bind/bind.hpp>
#include <boost/asio/placeholders.hpp>

#include "../abstract_server_unwrapped.hpp"
#include "./connection_unwrapped.hpp"

namespace hl
{
namespace net
{
    class udp_server_unwrapped : public base_abstract_server_unwrapped
    {
    public:
        using shared_t = std::shared_ptr<udp_server_unwrapped>;
        using udp_connection_t = udp_connection_unwrapped;
        using shared_udp_connection_t = typename udp_connection_t::shared_t;

    private:
        boost::asio::ip::udp::socket m_socket;
        boost::asio::ip::udp::endpoint m_endpoint;

        shared_buffer_t m_receive_buffer;

        void _receive_async_callback(const boost::system::error_code &ec, const size_t bytes_transferred)
        {
            shared_buffer_t buffer_cpy = make_shared_buffer(m_receive_buffer, bytes_transferred);

            if (ec)
            {
                HL_NET_LOG_WARN("Error on receive for server: {} with error: {}", get_alias(), ec.message());
                switch (ec.value())
                {
                // TODO: Better error handling
                _HL_INTERNAL_UNHEALTHY_CASES_CONNECTION_UNHEALTHY:
                _HL_INTERNAL_UNHEALTHY_CASES_SERVER_FROM_CONNECTION:
                    HL_NET_LOG_ERROR("Server cannot receive data due to {}, stopping receive, server is not healthy!", ec.message());
                    set_health_status(false);
                    break;
                default:
                    break;
                }
                connection_t connection(nullptr);
                callbacks_register().on_receive_error(connection, buffer_cpy, ec, bytes_transferred);
                _receive_async();
            }
            else
            {
                // TODO
                // possible that a race condition occurs here?
                // E.g getting the connection of a duplicate endpoint (same ip and port)
                // Should that even be possible? 
                // Should lock all the way between _get_connection and _set_connection?
                HL_NET_LOG_DEBUG("Received {} bytes from a client", bytes_transferred);
                const boost::asio::ip::udp::endpoint endpoint_cpy = m_endpoint;
                const std::string endpoint_str = utils::endpoint_to_string(endpoint_cpy);
                connection_t connection = _get_connection(endpoint_str);

                // connecting the client to the server 
                if (!connection)
                {
                    HL_NET_LOG_DEBUG("Connecting new client to server: {}", get_alias());

                    connection = std::static_pointer_cast<base_abstract_connection_unwrapped>(udp_connection_t::make(
                        callbacks_register(),
                        make_server_is_unhealthy_notifier(),
                        make_client_is_unhealthy_notifier(),
                        m_endpoint,
                        m_socket
                    ));
                    connection->set_alias(endpoint_str);
                    _set_connection(connection, utils::endpoint_to_string(m_endpoint));
                    callbacks_register().on_connection(connection);
                    HL_NET_LOG_DEBUG("Connected new client {} to server: {}", connection->get_id(), get_alias());
                }

                HL_NET_LOG_DEBUG("Received {} bytes from client: {} for server: {}", bytes_transferred, connection->get_id(), get_alias());

                callbacks_register().on_receive(connection, buffer_cpy, bytes_transferred);
                _receive_async();
            }
        }

        void _receive_async()
        {
            std::lock_guard<std::mutex> lock(m_mutex_api_control_flow);

            if (!healthy())
            {
                HL_NET_LOG_ERROR("Cannot read: Server is not healthy: {} may be either disconnected or received a non-recoverable error", get_alias());
                connection_t connection(nullptr);
                shared_buffer_t buffer(nullptr);
                callbacks_register().on_receive_error(connection, buffer, boost::system::error_code(boost::asio::error::not_connected), 0);
                return;
            }

            HL_NET_LOG_DEBUG("Start reading for server: {}", get_alias());

            m_socket.async_receive_from(
                boost::asio::buffer(*m_receive_buffer),
                m_endpoint,
                boost::bind(&udp_server_unwrapped::_receive_async_callback, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
            );
        }

        udp_server_unwrapped()
            : base_abstract_server_unwrapped()
            , m_socket(_io_service())
            , m_endpoint()
            , m_receive_buffer(make_shared_buffer())
        {
            HL_NET_LOG_TRACE("Creating udp_server_unwrapped: {}", get_alias());
        }

    public:
        static shared_t make()
        {
            return shared_t(new udp_server_unwrapped);
        }

        virtual ~udp_server_unwrapped()
        {
            HL_NET_LOG_TRACE("Destroying udp_server_unwrapped: {}", get_alias());
            stop();
            HL_NET_LOG_TRACE("Destroyed udp_server_unwrapped: {}", get_alias());
        }
    
    public:
        bool start(const std::string &port) override final
        {
            {
                std::lock_guard<std::mutex> lock_flow(m_mutex_api_control_flow);

                HL_NET_LOG_DEBUG("Starting server: {} on 0.0.0.0:{}", get_alias(), port);

                const int64_t i64port = std::stoll(port);
                const port_t    vport = static_cast<port_t>(i64port);

                if (i64port < MIN_PORT || i64port > MAX_PORT)
                {
                    HL_NET_LOG_ERROR("Invalid port given to the server for: ", get_alias());
                    return false;
                }

                if (is_running())
                {
                    HL_NET_LOG_ERROR("Server already started for: {}", get_alias());
                    return false;
                }

                boost::asio::ip::udp::endpoint const endpoint(boost::asio::ip::udp::v4(), vport);
                boost::system::error_code ec;

                m_socket.open(boost::asio::ip::udp::v4(), ec);
                if (ec)
                {
                    HL_NET_LOG_ERROR("Failed to open socket: {} for {}", ec.message(), get_alias());
                    return false;
                }
                m_socket.bind(endpoint, ec);
                if (ec)
                {
                    HL_NET_LOG_ERROR("Failed to bind socket for: {}", get_alias());
                    return false;
                }

                _unsafe_start();

                callbacks_register().on_start_success();
            }

            _receive_async();
            HL_NET_LOG_DEBUG("Started server: {} on 0.0.0.0:{}", get_alias(), port);
            return true;
        }

        bool stop() override final
        {
            std::lock_guard<std::mutex> lock_flow(m_mutex_api_control_flow);
            HL_NET_LOG_DEBUG("Stopping server: {}", get_alias());

            if (is_running() == false)
            {
                HL_NET_LOG_WARN("Server already stopped: {}", get_alias());
                callbacks_register().on_stop_error(boost::system::error_code(boost::asio::error::operation_aborted));
                return false;
            }

            m_socket.close();
            _unsafe_stop();
            HL_NET_LOG_DEBUG("Stopped server: {}", get_alias());
            return true;
        }
    };
}
}