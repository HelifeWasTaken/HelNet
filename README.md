# HelNet

Multithreaded utilities using boost::asio and modern C++ for client/server apps (TCP &amp; UDP)

## Requirements

- C++17
- Boost (Unknown minimum version yet)
- Boost Asio
- spdlog
- fmt

## Example of TCP Client

```cpp
#include "HelNet.hpp"

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        HL_NET_LOG_ERROR("Usage: {} <host> <tcp-port>", argv[0]);
        return 1;
    }

    hl::net::tcp_client client(argv[1], argv[2]);
    // While service is considered healthy
    // Equivalent to: client.healthy()
    while (client) { 
        client.send_string("Hello, World!");
    }
}
```

## Example of UDP Client

```cpp
#include "HelNet.hpp"

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        HL_NET_LOG_ERROR("Usage: {} <host> <tcp-port>", argv[0]);
        return 1;
    }

    hl::net::udp_client client(argv[1], argv[2]);
    // While service is considered healthy
    // Equivalent to: client.healthy()
    while (client) { 
        client.send_string("Hello, World!");
    }
}
```

## Client Interoperability

```cpp
#include "HelNet.hpp"

template<typename Protocol>
int hello_world_client(char const *host, char const *port)
{
    hl::net::client_wrapper<Protocol> client(host, port);
    while (client.healthy()) {
        client.send_string("Hello, World!");
    }
}

int main(int argc, char **argv)
{
    const std::string protocol = argc < 4 ? "tcp" : argv[3];

    if (argc != 4 && argc != 3)
    {
        HL_NET_LOG_ERROR("Usage: {} <host> <port> [protocol(tcp*/udp)]", argv[0]);
        return 1;
    }
    else if (protocol == "tcp")
    {
        return hello_world_client<hl::net::tcp_client_unwrapped>(argv[1], argv[2]); // TCP
    }
    else if (protocol == "udp")
    {
        return hello_world_client<hl::net::udp_client_unwrapped>(argv[1], argv[2]); // UDP
    }
    else
    {
        HL_NET_LOG_ERROR("Unknown protocol: {}", protocol);
        return 1;
    }
}
```

## Example of a TCP/UDP Server (Echo) & Client

**Disclaimer**: UDP is not implemented yet for listening

This example can start a client that connects to a server and can send messages to it by reading from stdin (blocking)... The client will handle the received data asynchronously and print it to stdout. The clients also leaves if `std::cin.bad()` is true (input closed).
This example can also start a server that listens for incoming connections and echoes back the received data asynchrnously to the client. The server will stop if it receives the message "exit" from a client.
All of the given examples are run asynchronously for receiving and sending data...
All of the callbacks are set to be synchronous by default, but can be set to be asynchronous by setting the `async` parameter to `true` in the `set_on_receive` and `set_on_send` functions.
The callbacks are synchronous by default because launching a new thread for each callbacks has been proven to be inefficient when the callbacks are simply logging the received data or sending it back to the client.

```cpp

#include "HelNet.hpp"
#include <iostream>

template<class Protocol>
void client_routine(const std::string& host, const std::string& port)
{
    Protocol client;

    if (client.connect(host, port) == false)
    {
        return;
    }

    // Write all received data to stdout asynchronously
    client.callbacks().set_on_receive([](
        hl::net::base_abstract_client_unwrapped&,
        hl::net::shared_buffer_t data,
        const size_t& size
    )
    {
        try {
            std::string str((const char*)(*data).data(), size);
            HL_NET_LOG_CRITICAL("Received: {}", str);
        } catch (const std::exception& e) {
            HL_NET_LOG_CRITICAL("Exception when printing: {}", e.what());
        }
    });
    client.callbacks().set_on_receive_async(true);

    while (client)
    {
        std::string line;
        std::getline(std::cin, line);
        if (std::cin.bad())
        {
            std::cout << "Input closed - exiting" << std::endl;
            break;
        }

        client.send_string(line);
    }
}

template<class Protocol>
void server_routine(const std::string& port)
{
    Protocol server;

    if (server.start(port) == false) {
        return;
    }

    //(base_abstract_server_unwrapped& server, shared_abstract_connection client,
    // shared_buffer_t buffer_copy, const size_t recv_bytes)>;

    std::atomic_bool run;

    server.callbacks().set_on_receive([&run = run](
        hl::net::base_abstract_server_unwrapped&,
        hl::net::shared_abstract_connection client,
        hl::net::shared_buffer_t data,
        const size_t& size
    )
    {
        try {
            std::string str((const char*)(*data).data(), size);
            if (str == "exit") {
                HL_NET_LOG_CRITICAL("Received: exit - closing server...");
                run = false;
                return;
            }

            HL_NET_LOG_CRITICAL("Received: {}, echoing back to client {}", str, client->get_id());
            client->send(data, size);
        } catch (const std::exception& e) {
            HL_NET_LOG_CRITICAL("Exception when printing: {} - closing server...", e.what());
            run = false;
        }
    });
    server.callbacks().set_on_receive_async(true);

    while (server && run)
    {
        // Sleep for a second to avoid 100% CPU usage (Will make the program wait for a second but it's fine for this example)
        std::this_thread::sleep_for(std::chrono::seconds(1)); 
    }
}

int main(int argc, char **argv)
{
    HL_NET_SETUP_LOG_LEVEL();

    std::string port, mode = "connect", protocol = "tcp", host = "localhost";

#if __GNUC__ || __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#endif
    switch (argc)
    {
    case 5:
        host = argv[4];
    case 4:
        protocol = argv[3];
    case 3:
        mode = argv[2];
    case 2:
        port = argv[1];
        break;
    default:
        std::cout << argv[0] << ": <port> [*connect|listen] [*tcp|udp] [host = localhost (ignored for listen)]" << std::endl;
        return 1;
    }
#if __GNUC__ || __clang__
#pragma GCC diagnostic pop
#endif

    if (protocol != "tcp" && protocol != "udp") {
        HL_NET_LOG_ERROR("Unsupported protocol: {} (only tcp/udp)", protocol);
        return 1;
    } else if (mode == "listen") {
        if (protocol == "tcp") {
            server_routine<hl::net::tcp_server>(port);
        } else {
            std::cerr << "UDP not implmeneted yet for listening" << std::endl;
            return 2;
        }
    } else if (mode == "connect") {
        protocol == "tcp" ? client_routine<hl::net::tcp_client>(host, port) : client_routine<hl::net::udp_client>(host, port);
    } else {
        std::cout << "Unsupported mode (only connect/listen)" << std::endl;
    }
    return 0;
}
```
