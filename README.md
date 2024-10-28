# HelNet

Multithreaded utilities using boost::asio and modern C++ for client/server apps (TCP &amp; UDP)

## Requirements

- C++17
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

    // While service is considered healthy
    // Equivalent to: client.healthy()
    while (client) { 
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
        if (str == "exit" || str == "exit\n" || str == "exit\r\n") { // Handle nc and telnet style
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

    server.callbacks().set_on_receive(std::bind(handle_on_receive, std::ref(server), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    server.callbacks().set_on_receive_async(true);

    while (server) {
        // Sleep for a second to avoid 100% CPU usage (Will make the program wait for a second but it's fine for this example)
        std::this_thread::sleep_for(std::chrono::seconds(1)); 
    }
}
```

Why should i use this instead of boost::asio directly?

- It's somewhat easier to use
- It's more modern C++ friendly
- It has a logging system
- It handles connection states for you in TCP
- It recognizes clients in UDP
- It has a callback system for most of the events
- It will have a plugin system for callbacks (WIP*)
