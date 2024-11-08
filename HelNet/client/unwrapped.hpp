/*
This code is licensed under the GNU GPL v3.
Copyright: (C) 2024 Mattis DALLEAU
*/

#pragma once

#include "HelNet/client/callbacks.hpp"
#include "HelNet/utils.hpp"
#include <boost/asio/io_service.hpp>

namespace hl
{
namespace net
{
    // base client class
HL_NET_DIAGNOSTIC_PUSH()
HL_NET_DIAGNOSTIC_NON_VIRTUAL_DESTRUCTOR_IGNORED()
    class base_abstract_client_unwrapped : public std::enable_shared_from_this<base_abstract_client_unwrapped>, public hl::silva::collections::meta::NonCopyMoveable
    {
HL_NET_DIAGNOSTIC_POP()
    public:
        using shared_t = client_t;

    public:
        virtual bool connect(const std::string &host, const std::string &port) = 0;
        virtual bool disconnect(void) = 0;
        virtual bool send(const shared_buffer_t &buffer, const size_t &size) = 0;

    private:
        std::atomic_bool m_connected;
        std::atomic_bool m_healthy;

        shared_buffer_t m_receive_buffer;

        mutable std::mutex m_alias_mutex;
        std::string m_alias;

        client_callback_register m_callback_register;

    protected:
        shared_buffer_t receive_buffer()
        {
            return this->m_receive_buffer;
        }

    protected:
        base_abstract_client_unwrapped()
            : m_connected(false)
            , m_healthy(false)
            , m_receive_buffer(make_shared_buffer())
            , m_alias_mutex()
            , m_alias(fmt::format("base_abstract_client_unwrapped({})", static_cast<void *>(this)))
            , m_callback_register([this]() -> client_t { return this->as_sharable(); })
        {}

    public:
        const std::string get_alias() const
        {
            std::lock_guard<std::mutex> lock(this->m_alias_mutex);
            return std::string(this->m_alias);
        }

        void set_alias(const std::string &alias)
        {
            const std::string alias_copy = this->get_alias();
            std::lock_guard<std::mutex> lock(this->m_alias_mutex);
            HL_NET_LOG_INFO("Set alias for client: {} to: {}", alias_copy, alias);
            this->m_alias = alias;
        }

        virtual ~base_abstract_client_unwrapped() override
        {
            HL_NET_LOG_TRACE("Destroying base_client_unwrapped: {}", this->get_alias());
            HL_NET_LOG_TRACE("Destroyed base_client_unwrapped: {}", this->get_alias());
        }

        client_callback_register& callbacks_register()
        {
            return this->m_callback_register;
        }

        bool connected(void) const
        {
            return this->m_connected;
        }

        bool healthy(void) const
        {
            return this->m_healthy && this->connected();
        }

        void set_connect_status(const bool status)
        {
            this->m_connected = status;
            HL_NET_LOG_WARN("Client: {} connected status set to: {}", this->get_alias(), status);
        }

        void set_health_status(const bool status)
        {
            this->m_healthy = status;
            HL_NET_LOG_WARN("Client: {} health status set to: {}", this->get_alias(), status);
        }

        bool send(const shared_buffer_t &buffer)
        {
            return this->send(buffer, buffer->size());
        }

        bool send_bytes(const byte *data, const size_t &size)
        {
            shared_buffer_t shared_buffer = make_shared_buffer(data, size);
            return this->send(shared_buffer, size);
        }

        template<typename T>
        bool send_bytes(const std::vector<T> &data)
        {
            return this->send_bytes(static_cast<const byte *>(data.data()), data.size() * sizeof(T));
        }

        bool send_string(const std::string &str)
        {
            return this->send_bytes(reinterpret_cast<const byte *>(str.c_str()), str.size());
        }

        client_t as_sharable()
        {
            HL_NET_LOG_CRITICAL("Asked for sharable client: {}", this->get_alias());
            return shared_from_this();
        }
    };

    template<class Protocol>
    class base_client_unwrapped final : public base_abstract_client_unwrapped
    {
    public:
        using shared_t = boost::shared_ptr<base_client_unwrapped<Protocol>>;
        using this_type_t = base_client_unwrapped<Protocol>;

        struct connection_data 
        {
            boost::asio::io_service io_service;

            typename Protocol::resolver resolver;
            typename Protocol::socket socket;
            typename Protocol::resolver::iterator endpoint_iterator;

            std::thread io_service_thread;

            connection_data()
                : io_service()
                , resolver(io_service)
                , socket(io_service)
                , endpoint_iterator()
                , io_service_thread()
            {
            }

            connection_data(const connection_data &) = delete;
            connection_data &operator=(const connection_data &) = delete;
            connection_data(connection_data &&) = delete;
            connection_data &operator=(connection_data &&) = delete;

            ~connection_data() = default;

            void resolve(const std::string &host, const std::string &port)
            {
                this->endpoint_iterator = this->resolver.resolve({ host, port });
            }

            template<typename P = Protocol, utils::enable_if_t<utils::is_same<P, boost::asio::ip::tcp>::value>* = nullptr>
            boost::system::error_code connect(const std::string& host, const std::string &port)
            {
                this->resolve(host, port);

                boost::system::error_code error = boost::asio::error::host_not_found;
                typename Protocol::resolver::iterator end;

                while (error && this->endpoint_iterator != end)
                {
                    this->socket.close();
                    this->socket.connect(*this->endpoint_iterator++, error);
                }

                return error;
            }

            template<typename P = Protocol, utils::enable_if_t<utils::is_same<P, boost::asio::ip::udp>::value>* = nullptr>
            boost::system::error_code connect(const std::string& host, const std::string &port)
            {
                this->resolve(host, port);

                boost::system::error_code error = boost::asio::error::host_not_found;
                typename Protocol::resolver::iterator end;

                this->socket.open(Protocol::v4());
                this->socket.connect(*this->endpoint_iterator, error);
                return error;
            }
        };

    private:
        connection_data m_connection_data;
        std::mutex m_mutex_api_control_flow;

        void _receive_async_callback(const boost::system::error_code& ec, const size_t &bytes_transferred, const shared_buffer_t &buffer)
        {
            shared_buffer_t buffer_cpy = make_shared_buffer(buffer, bytes_transferred);

            HL_NET_LOG_DEBUG("Received {} bytes for client: {}", bytes_transferred, this->get_alias());
            if (ec)
            {
                HL_NET_LOG_WARN("Error on receive for client: {} with error: {}", this->get_alias(), ec.message());
                switch (ec.value())
                {
                _HL_INTERNAL_UNHEALTHY_CASES_CLIENT:
                    HL_NET_LOG_ERROR("Client cannot connect: {} stopping reading due to {}, stopping read, considered not healthy!", this->get_alias(), ec.message());
                    this->set_health_status(false);
                default:
                    break;
                }
                this->callbacks_register().on_receive_error(buffer_cpy, ec, bytes_transferred);
            }
            else
            {
                this->callbacks_register().on_receive(buffer_cpy, bytes_transferred);
            }
            this->_receive_async();
        }

        void _receive_async()
        {
            std::lock_guard<std::mutex> lock_flow(m_mutex_api_control_flow);

            if (this->healthy() == false)
            {
                HL_NET_LOG_ERROR("Cannot read: client is not healthy: {} may be either disconnected or received a non-recoverable error", this->get_alias());
                shared_buffer_t recv_buffer = this->receive_buffer();
                this->callbacks_register().on_receive_error(recv_buffer, boost::asio::error::not_connected, 0);
                return;
            }

            shared_buffer_t recv_buffer = this->receive_buffer();

            HL_NET_LOG_TRACE("Start reading for client: {}", this->get_alias());

            m_connection_data.socket.async_receive(
                boost::asio::buffer(*recv_buffer),
                [this, recv_buffer]
                (const boost::system::error_code &ec, const size_t &bytes_transferred) -> void
                {
                    this->_receive_async_callback(ec, bytes_transferred, recv_buffer);
                }
            );
        }

        base_client_unwrapped()
            : m_connection_data()
            , m_mutex_api_control_flow()
        {
            HL_NET_LOG_TRACE("Created base_client_unwrapped: {}", this->get_alias());
        }

        void basic_io_service_coroutine()
        {
            HL_NET_LOG_TRACE("Starting io_service coroutine for: {}", get_alias());
            while (healthy())
            {
                m_connection_data.io_service.run();
                HL_NET_LOG_TRACE("Restarting io_service coroutine for: {}", get_alias());
                m_connection_data.io_service.reset();
            }
            HL_NET_LOG_TRACE("Stopping io_service coroutine for: {}", get_alias());
        }

    public:
        static shared_t make()
        {
            return shared_t(new this_type_t);
        }

        // Moved on the base class the de
        virtual ~base_client_unwrapped() override final
        {
            HL_NET_LOG_TRACE("Destroying base_client_unwrapped: {}", this->get_alias());
            if (this->connected())
            {
                this->disconnect();
            }
            HL_NET_LOG_TRACE("Destroyed base_client_unwrapped: {}", this->get_alias());
        }

        virtual bool connect(const std::string &host, const std::string &port) override final
        {
            {
                std::lock_guard<std::mutex> lock(this->m_mutex_api_control_flow);

                HL_NET_LOG_DEBUG("Connecting client: {} to {}:{}", this->get_alias(), host, port);

                if (this->connected() == true)
                {
                    HL_NET_LOG_ERROR("Client already connected: {}", this->get_alias());
                    return false;
                }

                if (const boost::system::error_code error = this->m_connection_data.template connect<Protocol>(host, port))
                {
                    HL_NET_LOG_ERROR("Error connecting client: {} with error: {}", this->get_alias(), error.message());
                    return false;
                }

                client_callback_register &callback_register = this->callbacks_register();

                callback_register.unsafe_start_pool();
                this->set_connect_status(true);
                this->set_health_status(true);

                this->m_connection_data.io_service_thread = std::thread(std::bind(&this_type_t::basic_io_service_coroutine, this));

                callback_register.on_connect();
                HL_NET_LOG_DEBUG("Connected client: {}", this->get_alias());
            }

            this->_receive_async();
            return true;
        }

        virtual bool disconnect() override final
        {
            std::lock_guard<std::mutex> lock(this->m_mutex_api_control_flow);

            HL_NET_LOG_DEBUG("Disconnecting client: {}", this->get_alias());

            if (this->connected() == false)
            {
                HL_NET_LOG_WARN("Client already disconnected: {}", this->get_alias());
                this->callbacks_register().on_disconnect_error(boost::asio::error::not_connected);
                return false;
            }

            this->set_health_status(false);

            this->m_connection_data.socket.close();
            this->m_connection_data.io_service.stop();
            this->m_connection_data.io_service_thread.join();

            client_callback_register &callback_register = this->callbacks_register();

            callback_register.on_disconnect();

            HL_NET_LOG_DEBUG("Disconnected client: {}", this->get_alias());

            callback_register.unsafe_stop_pool();

            set_connect_status(false);

            return true;
        }
    
    private:
        void _send_async_callback(const boost::system::error_code &ec, size_t bytes_transferred)
        {
            HL_NET_LOG_DEBUG("Sent {} bytes for client: {}", bytes_transferred, this->get_alias());

            if (ec)
            {
                HL_NET_LOG_WARN("Error on send for client: {} with error: {} and {} bytes", this->get_alias(), ec.message(), bytes_transferred);
                this->callbacks_register().on_send_error(ec, bytes_transferred);

                switch (ec.value())
                {
                _HL_INTERNAL_UNHEALTHY_CASES_CLIENT:
                    HL_NET_LOG_ERROR("Client cannot send data: {} due to {}, stopping send, considered not healthy!", get_alias(), ec.message());
                    this->set_health_status(false);
                    break;
                default:
                    break;
                }
            }
            else
            {
                this->callbacks_register().on_sent(bytes_transferred);
            }
        }

        template<typename P = Protocol, utils::enable_if_t<utils::is_same<P, boost::asio::ip::tcp>::value>* = nullptr>
        inline void _send_async_protocol(const shared_buffer_t &buffer, const size_t &size)
        {
            this->m_connection_data.socket.async_send(
                boost::asio::buffer(*buffer, size),
                [this, buffer](const boost::system::error_code &ec, const size_t &bytes_transferred) -> void
                {
                    this->_send_async_callback(ec, bytes_transferred);
                }
            );
        }

        template<typename P = Protocol, utils::enable_if_t<utils::is_same<P, boost::asio::ip::udp>::value>* = nullptr>
        inline void _send_async_protocol(const shared_buffer_t &buffer, const size_t &size)
        {
            this->m_connection_data.socket.async_send_to(
                boost::asio::buffer(*buffer, size),
                *this->m_connection_data.endpoint_iterator,
                [this, buffer](const boost::system::error_code &ec, const size_t &bytes_transferred) -> void
                {
                    this->_send_async_callback(ec, bytes_transferred);
                }
            );
        }

    public:
        virtual bool send(const shared_buffer_t &buffer, const size_t &size) override final
        {
            std::lock_guard<std::mutex> lock(this->m_mutex_api_control_flow);

            HL_NET_LOG_TRACE("Preparing to send {} bytes for client: {}", size, this->get_alias());

            if (!healthy())
            {
                HL_NET_LOG_ERROR("Cannot send data to from a non-healthy client: {}", this->get_alias());
                this->callbacks_register().on_send_error(boost::system::error_code(boost::asio::error::not_connected), 0);
                return false;
            }
            else if (!size)
            {
                HL_NET_LOG_ERROR("Cannot send 0 bytes to the client: {}", this->get_alias());
                this->callbacks_register().on_send_error(boost::system::error_code(boost::asio::error::invalid_argument), 0);
                return false;
            }
            else if (!buffer)
            {
                HL_NET_LOG_ERROR("Cannot send data from a null buffer client: {}", this->get_alias());
                this->callbacks_register().on_send_error(boost::system::error_code(boost::asio::error::invalid_argument), 0);
                return false;
            }
            else if (size > buffer->size())
            {
                HL_NET_LOG_ERROR("Cannot send more than the buffer size: {} bytes from client: {}", buffer->size(), this->get_alias());
                this->callbacks_register().on_send_error(boost::system::error_code(boost::asio::error::message_size), 0);
                return false;
            }
            else
            {
                HL_NET_LOG_DEBUG("Sending {} bytes for client: {}", size, this->get_alias());
                _send_async_protocol<Protocol>(buffer, size);
                return true;
            }
        }
    };

}
}
