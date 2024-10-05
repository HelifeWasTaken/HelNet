# HelNet

Multithreaded utilities using boost::asio and modern C++ for client/server apps (TCP &amp; UDP)

## Example of TCP Client

```cpp
#include "HelNet"

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        spdlog::error("Usage: {} <host> <tcp-port>", argv[0]);
        return 1;
    }

    try {
        hl::net::tcp_client_wrapper client_wrapper(argv[1], argv[2]);
        while (true) {
            client_wrapper.client().send_string("Hello, World!");
        }
    } catch (const hl::net::tcp_client_wrapper::Error &error) {
        SPDLOG_CRITICAL("Catched Error: {}", error.what());
        return 1;
    }
}
```

## Example of UDP Client

```cpp
#include "HelNet"

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        spdlog::error("Usage: {} <host> <tcp-port>", argv[0]);
        return 1;
    }

    try {
        hl::net::udp_client_wrapper client_wrapper(argv[1], argv[2]);
        while (true) {
            client_wrapper.client().send_string("Hello, World!");
        }
    } catch (const hl::net::tcp_client_wrapper::Error &error) {
        SPDLOG_CRITICAL("Catched Error: {}", error.what());
        return 1;
    }
}
```

## Client Interoperability

```cpp
#include "HelNet"

using namespace hl::net;

template<typename Protocol>
int hello_world_client()
{
    try {
        client_wrapper<Protocol> client_wrapper(argv[1], argv[2]);
        while (true) {
            client_wrapper.client().send_string("Hello, World!");
        }
    } catch (const hl::net::tcp_client_wrapper::Error &error) {
        SPDLOG_CRITICAL("Catched Error: {}", error.what());
        return 1;
    }
}

int main(int argc, char **argv)
{
    const std::string protocol = argc < 4 ? "tcp" : argv[3];

    if (argc < 3)
    {
        spdlog::error("Usage: {} <host> <tcp-port> [protocol(tcp*/udp)]", argv[0]);
        return 1;
    }

    if (protocol == "tcp")
        return hello_world_client<tcp_client>(); // TCP
    else if (protocol == "udp")
        return hello_world_client<udp_client>(); // UDP
    else
        spdlog::error("Unknown protocol: {}", argv[3]);
        return 1;
}
```
