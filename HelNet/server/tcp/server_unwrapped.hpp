/*
This code is licensed under the GNU GPL v3.
Copyright: (C) 2024 Mattis DALLEAU
*/

#pragma once

#include <boost/asio/ip/tcp.hpp>
#include "../abstract_server_unwrapped.hpp"
#include "./connection_unwrapped.hpp"
#include "../utils.hpp"

namespace hl
{
namespace net
{
    class tcp_server_unwrapped : public base_abstract_server_unwrapped
    {
    public:
        using shared_t = std::shared_ptr<tcp_server_unwrapped>;
        using tcp_connection_t = tcp_connection_unwrapped;
        using shared_tcp_connection_t = typename tcp_connection_t::shared_t;

    private:
        boost::asio::ip::tcp::acceptor m_acceptor;

        void _async_accept_callback(const boost::system::error_code &ec, shared_tcp_connection_t connection)
        {
            if (ec)
            {
                HL_NET_LOG_WARN("Error on accept for server: {} with error: {}", get_alias(), ec.message());
                switch (ec.value())
                {
                _HL_INTERNAL_UNHEALTHY_CASES_FOR_SERVER:
                    HL_NET_LOG_ERROR("Connection cannot accept data from: {} due to {}, stopping receive, server is not healthy!", get_alias(), ec.message());
                    this->set_health_status(false);
                    break;
                default:
                    break;
                }
                callbacks_register().on_connection_error(ec);
            }
            else
            {
                HL_NET_LOG_DEBUG("Accepted connection for server: {}", get_alias());
                base_abstract_connection_unwrapped::shared_t conn_callback = std::static_pointer_cast<base_abstract_connection_unwrapped>(connection);
                _set_connection(conn_callback, utils::endpoint_to_string(connection->socket().remote_endpoint()));
                connection->start_receive();
                callbacks_register().on_connection(conn_callback);
            }
            _accept_async();
        }

        void _accept_async()
        {
            if (!healthy())
            {
                HL_NET_LOG_ERROR("Cannot read: Server is not healthy: {} may be either disconnected or received a non-recoverable error", get_alias());
                callbacks_register().on_connection_error(boost::asio::error::not_connected);
                return;
            }

            std::lock_guard<std::mutex> lock_flow(m_mutex_api_control_flow);

            shared_tcp_connection_t connection = tcp_connection_t::make(
                _io_service(),
                callbacks_register(),
                make_server_is_unhealthy_notifier(),
                make_client_is_unhealthy_notifier()
            );

            m_acceptor.async_accept(
                connection->socket(),
                [this, connection](const boost::system::error_code &ec) -> void { this->_async_accept_callback(ec, connection); }
            );
        }

        tcp_server_unwrapped()
            : base_abstract_server_unwrapped()
            , m_acceptor(_io_service())
        {
            HL_NET_LOG_TRACE("Creating tcp_server_unwrapped: {}", get_alias());
        }

    public:
        static shared_t make()
        {
            return shared_t(new tcp_server_unwrapped);
        }

        virtual ~tcp_server_unwrapped()
        {
            HL_NET_LOG_TRACE("Destroying tcp_server_unwrapped: {}", get_alias());
            stop();
            HL_NET_LOG_TRACE("Destroyed tcp_server_unwrapped: {}", get_alias());
        }

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

                boost::asio::ip::tcp::endpoint const endpoint(boost::asio::ip::tcp::v4(), vport);

                boost::system::error_code ec;

                m_acceptor.open(boost::asio::ip::tcp::v4(), ec);
                if (ec)
                {
                    HL_NET_LOG_ERROR("Failed to open acceptor: {} for {}", ec.message(), get_alias());
                    return false;
                }

                m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
                if (ec)
                {
                    HL_NET_LOG_ERROR("Failed to set reuse address on acceptor: {} for {}", ec.message(), get_alias());
                    return false;
                }

                m_acceptor.bind(endpoint, ec);
                if (ec)
                {
                    HL_NET_LOG_ERROR("Failed to bind acceptor for: {}", get_alias());
                    return false;
                }

                m_acceptor.listen(MAX_CONNECTIONS, ec);
                if (ec)
                {
                    HL_NET_LOG_ERROR("Failed to listen acceptor for: {}", get_alias());
                    return false;
                }

                _unsafe_start();
            }

            _accept_async();

            callbacks_register().on_start_success();
            HL_NET_LOG_DEBUG("Server ready: {} on 0.0.0.0:{}", get_alias(), port);
            return true;
        }

        bool stop() override final
        {
            std::lock_guard<std::mutex> lock_flow(m_mutex_api_control_flow);
            HL_NET_LOG_DEBUG("Stopping server: {}", get_alias());

            if (is_running() == false)
            {
                HL_NET_LOG_WARN("Tried to stop already stopped server: {}", get_alias());
                callbacks_register().on_stop_error(boost::asio::error::not_connected);
                return false;
            }

            m_acceptor.close();
            _unsafe_stop();

            HL_NET_LOG_DEBUG("Stopped server: {}", get_alias());
            return true;
        }
    };
}
}