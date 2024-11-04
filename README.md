# HelNet

Multithreaded utilities using boost::asio and modern C++ for client/server apps (TCP &amp; UDP)

## Requirements

- C++11
- Boost (Unknown minimum version yet)
- Boost Asio
- spdlog
- fmt

## Example of TCP/UDP Client

```cpp
#include "HelNet.hpp"

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        HL_NET_LOG_ERROR("Usage: {} <host> <port>", argv[0]);
        return 1;
    }

    hl::net::tcp_client client(argv[1], argv[2]);
    // hl::net::udp_client client(argv[1], argv[2]);

    while (client.update()) { 
        client.send_string("Hello, World!");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
```

**Disclaimer:** UDP does not support `hostnames` as it does not have a connection, it only sends data to the given address and port. It means that `localhost` will not work for UDP but it will work for TCP.

## Example of a TCP/UDP Server

**Disclaimer:**

As UDP does not have connection but we have connected endpoint, the user is solely responsible for handling the connection state. For example a wrapper for handling timeouts and reconnection attempts! To make it easier, the user will soon be able to **attach** plugins to callbacks before they are called! Timeouts can be done with `boost::asio::deadline_timer`.

UDP does not support `hostnames` as it does not have a connection, it only sends data to the given address and port. It means that `localhost` will not work for UDP but it will work for TCP.

The server will consider the connection healthy for UDP as long as the server is running and a `connection` in it purest form never fails. A connection is done as soon as the server receives data from a client.

```cpp
// Auto setup logging at the minimum level
#define HL_NET_LOG_LEVEL HL_NET_LOG_LEVEL_MIN
#define HL_NET_AUTO_SETUP_LOG_SERVICE

#include "HelNet.hpp"

// using log critical to make sure the message is visible and printed
static void server_handle_on_receive(hl::net::base_abstract_server_unwrapped& server,
                              hl::net::connection_t client,
                              hl::net::shared_buffer_t buffer,
                              const size_t& size)
{
    try {
        std::string str(reinterpret_cast<char*>(buffer->data()), size);
        str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));

        if (str == "exit") { // Handle nc and telnet style
            HL_NET_LOG_CRITICAL("Received: exit - closing server...");
            server.stop();
            return;
        }
        HL_NET_LOG_CRITICAL("Received: {}, echoing back to client {}", str, client->get_id());
        client->send(buffer, size);
    } catch (const std::exception& e) {
        HL_NET_LOG_CRITICAL("Exception when printing: {} - closing server...", e.what());
        server.stop();
    }
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        HL_NET_LOG_ERROR("Usage: {} <port>", argv[0]);
        return 1;
    }

    hl::net::tcp_server server;
    // hl::net::udp_server server; // Decomment this line to use UDP

    if (server.start(argv[1]) == false) {
        return 1;
    }

    // When udp to disconnect automatically the clients because clients won't disconnect automatically
    // server.template attach_plugin<hl::net::plugins::server_clients_timeout>(2000); // 2000ms

    server.callbacks().set_on_receive(std::bind(handle_on_receive, std::ref(server), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    server.callbacks().set_on_receive_async(true);

    while (server.update()) {
        // Sleep for a second to avoid 100% CPU usage (Will make the program wait for a second but it's fine for this example)
        std::this_thread::sleep_for(std::chrono::seconds(1)); 
    }
}
```

## Clients callbacks

```cpp
namespace hl::net
{

using client_on_connect_callback            = std::function<void(client_t client)>;
using client_on_disconnect_callback         = std::function<void(void)>;
using client_on_disconnect_error_callback   = std::function<void(const boost::system::error_code &ec)>;
using client_on_receive_callback            = std::function<void(client_t client, shared_buffer_t buffer_copy, const size_t recv_bytes)>;
using client_on_receive_error_callback      = std::function<void(client_t client, shared_buffer_t buffer_copy, const boost::system::error_code ec, const size_t recv_bytes)>;
using client_on_sent_callback               = std::function<void(client_t client, const size_t sent_bytes)>;
using client_on_send_error_callback         = std::function<void(client_t client, const boost::system::error_code ec, const size_t sent_bytes)>;

struct client_callbacks final {
    client_on_connect_callback          on_connect_callback = nullptr;
    bool                                on_connect_is_async = false;

    client_on_disconnect_callback       on_disconnect_callback = nullptr;
    bool                                on_disconnect_is_async = false;

    client_on_disconnect_error_callback on_disconnect_error_callback = nullptr;
    bool                                on_disconnect_error_is_async = false;

    client_on_receive_callback          on_receive_callback = nullptr;
    bool                                on_receive_is_async = false;

    client_on_receive_error_callback    on_receive_error_callback = nullptr;
    bool                                on_receive_error_is_async = false;

    client_on_sent_callback             on_sent_callback = nullptr;
    bool                                on_sent_is_async = false;

    client_on_send_error_callback       on_send_error_callback = nullptr;
    bool                                on_send_error_is_async = false;
};

#define HL_NET_CLIENT_ON_CONNECT(CLIENT) [](client_t CLIENT)
#define HL_NET_CLIENT_ON_CONNECT_CAPTURE(CLIENT, ...) [__VA_ARGS__](client_t CLIENT)
#define HL_NET_CLIENT_ON_DISCONNECT() []() 
#define HL_NET_CLIENT_ON_DISCONNECT_CAPTURE(...) [__VA_ARGS__]()
#define HL_NET_CLIENT_ON_DISCONNECT_ERROR(EC) [](const boost::system::error_code &EC)
#define HL_NET_CLIENT_ON_DISCONNECT_ERROR_CAPTURE(EC, ...) [__VA_ARGS__](const boost::system::error_code &EC)
#define HL_NET_CLIENT_ON_RECEIVE(CLIENT, BUFFER_COPY, RECV_BYTES) [](client_t CLIENT, shared_buffer_t BUFFER_COPY, const size_t RECV_BYTES) 
#define HL_NET_CLIENT_ON_RECEIVE_CAPTURE(CLIENT, BUFFER_COPY, RECV_BYTES, ...) [__VA_ARGS__](client_t CLIENT, shared_buffer_t BUFFER_COPY, const size_t RECV_BYTES)
#define HL_NET_CLIENT_ON_RECEIVE_ERROR(CLIENT, BUFFER_COPY, EC, RECV_BYTES) [](client_t CLIENT, shared_buffer_t BUFFER_COPY, const boost::system::error_code EC, const size_t RECV_BYTES)
#define HL_NET_CLIENT_ON_RECEIVE_ERROR_CAPTURE(CLIENT, BUFFER_COPY, EC, RECV_BYTES, ...) [__VA_ARGS__](client_t CLIENT, shared_buffer_t BUFFER_COPY, const boost::system::error_code EC, const size_t RECV_BYTES)
#define HL_NET_CLIENT_ON_SENT(CLIENT, SENT_BYTES) [](client_t CLIENT, const size_t SENT_BYTES)
#define HL_NET_CLIENT_ON_SENT_CAPTURE(CLIENT, SENT_BYTES, ...) [__VA_ARGS__](client_t CLIENT, const size_t SENT_BYTES)
#define HL_NET_CLIENT_ON_SEND_ERROR(CLIENT, EC, SENT_BYTES) [](client_t CLIENT, const boost::system::error_code EC, const size_t SENT_BYTES)
#define HL_NET_CLIENT_ON_SEND_ERROR_CAPTURE(CLIENT, EC, SENT_BYTES, ...) [__VA_ARGS__](client_t CLIENT, const boost::system::error_code EC, const size_t SENT_BYTES)

}
```

### Server callbacks

```cpp
namespace hl::net
{

using server_on_start_success_callback          = std::function<void(server_t server)>;

using server_on_stop_success_callback           = std::function<void()>;
using server_on_stop_error_callback             = std::function<void(const boost::system::error_code ec)>;

using server_on_connection_callback             = std::function<void(server_t server, connection_t client)>;
using server_on_connection_error_callback       = std::function<void(server_t server, const boost::system::error_code ec)>;

using server_on_disconnection_callback          = std::function<void(server_t server, const client_id_t& client)>;
using server_on_disconnection_error_callback    = std::function<void(server_t server, const boost::system::error_code ec)>;

using server_on_sent_callback                   = std::function<void(server_t server, connection_t client, const size_t sent_bytes)>;
using server_on_send_error_callback             = std::function<void(server_t server, connection_t client, const boost::system::error_code ec, const size_t sent_bytes)>;

using server_on_receive_callback                = std::function<void(server_t server, connection_t client, shared_buffer_t buffer_copy, const size_t recv_bytes)>;
using server_on_receive_error_callback          = std::function<void(server_t server, connection_t client, shared_buffer_t buffer_copy, const boost::system::error_code ec, const size_t recv_bytes)>;

struct server_callbacks final {
    server_on_start_success_callback    on_start_success_callback = nullptr;
    bool                                on_start_success_is_async = false;

    server_on_stop_success_callback     on_stop_success_callback = nullptr;
    bool                                on_stop_success_is_async = false;

    server_on_stop_error_callback       on_stop_error_callback = nullptr;
    bool                                on_stop_error_is_async = false;

    server_on_connection_callback       on_connection_callback = nullptr;
    bool                                on_connection_is_async = false;

    server_on_connection_error_callback on_connection_error_callback = nullptr;
    bool                                on_connection_error_is_async = false;

    server_on_disconnection_callback    on_disconnection_callback = nullptr;
    bool                                on_disconnection_is_async = false;

    server_on_disconnection_error_callback on_disconnection_error_callback = nullptr;
    bool                                   on_disconnection_error_is_async = false;

    server_on_sent_callback             on_sent_callback = nullptr;
    bool                                on_sent_is_async = false;

    server_on_send_error_callback       on_send_error_callback = nullptr;
    bool                                on_send_error_is_async = false;

    server_on_receive_callback          on_receive_callback = nullptr;
    bool                                on_receive_is_async = false;

    server_on_receive_error_callback    on_receive_error_callback = nullptr;
    bool                                on_receive_error_is_async = false;

    server_callbacks() = default;
    ~server_callbacks() = default;
};

#define HL_NET_SERVER_ON_START(SERVER) [](server_t SERVER)
#define HL_NET_SERVER_ON_START_CAPTURE(SERVER, ...) [__VA_ARGS__](server_t SERVER)
#define HL_NET_SERVER_ON_STOP() []()
#define HL_NET_SERVER_ON_STOP_CAPTURE(...) [__VA_ARGS__]()
#define HL_NET_SERVER_ON_STOP_ERROR(EC) [](const boost::system::error_code &EC)
#define HL_NET_SERVER_ON_STOP_ERROR_CAPTURE(EC, ...) [__VA_ARGS__](const boost::system::error_code &EC)
#define HL_NET_SERVER_ON_CONNECTION(SERVER, CLIENT) [](server_t SERVER, connection_t CLIENT)
#define HL_NET_SERVER_ON_CONNECTION_CAPTURE(SERVER, CLIENT, ...) [__VA_ARGS__](server_t SERVER, connection_t CLIENT)
#define HL_NET_SERVER_ON_CONNECTION_ERROR(SERVER, EC) [](server_t SERVER, const boost::system::error_code &EC)
#define HL_NET_SERVER_ON_CONNECTION_ERROR_CAPTURE(SERVER, EC, ...) [__VA_ARGS__](server_t SERVER, const boost::system::error_code &EC)
#define HL_NET_SERVER_ON_DISCONNECTION(SERVER, CLIENT_ID) [](server_t SERVER, const client_id_t &CLIENT_ID)
#define HL_NET_SERVER_ON_DISCONNECTION_CAPTURE(SERVER, CLIENT_ID, ...) [__VA_ARGS__](server_t SERVER, const client_id_t &CLIENT_ID)
#define HL_NET_SERVER_ON_DISCONNECTION_ERROR(SERVER, EC) [](server_t SERVER, const boost::system::error_code &EC)
#define HL_NET_SERVER_ON_DISCONNECTION_ERROR_CAPTURE(SERVER, EC, ...) [__VA_ARGS__](server_t SERVER, const boost::system::error_code &EC)
#define HL_NET_SERVER_ON_SENT(SERVER, CLIENT, SENT_BYTES) [](server_t SERVER, connection_t CLIENT, const size_t SENT_BYTES)
#define HL_NET_SERVER_ON_SENT_CAPTURE(SERVER, CLIENT, SENT_BYTES, ...) [__VA_ARGS__](server_t SERVER, connection_t CLIENT, const size_t SENT_BYTES)
#define HL_NET_SERVER_ON_SEND_ERROR(SERVER, CLIENT, EC, SENT_BYTES) [](server_t SERVER, connection_t CLIENT, const boost::system::error_code &EC, const size_t SENT_BYTES)
#define HL_NET_SERVER_ON_SEND_ERROR_CAPTURE(SERVER, CLIENT, EC, SENT_BYTES, ...) [__VA_ARGS__](server_t SERVER, connection_t CLIENT, const boost::system::error_code &EC, const size_t SENT_BYTES)
#define HL_NET_SERVER_ON_RECEIVE(SERVER, CLIENT, BUFFER_COPY, RECV_BYTES) [](server_t SERVER, connection_t CLIENT, shared_buffer_t BUFFER_COPY, const size_t RECV_BYTES)
#define HL_NET_SERVER_ON_RECEIVE_CAPTURE(SERVER, CLIENT, BUFFER_COPY, RECV_BYTES, ...) [__VA_ARGS__](server_t SERVER, connection_t CLIENT, shared_buffer_t BUFFER_COPY, const size_t RECV_BYTES)
#define HL_NET_SERVER_ON_RECEIVE_ERROR(SERVER, CLIENT, BUFFER_COPY, EC, RECV_BYTES) [](server_t SERVER, connection_t CLIENT, shared_buffer_t BUFFER_COPY, const boost::system::error_code &EC, const size_t RECV_BYTES)
#define HL_NET_SERVER_ON_RECEIVE_ERROR_CAPTURE(SERVER, CLIENT, BUFFER_COPY, EC, RECV_BYTES, ...) [__VA_ARGS__](server_t SERVER, connection_t CLIENT, shared_buffer_t BUFFER_COPY, const boost::system::error_code &EC, const size_t RECV_BYTES)


}
```
