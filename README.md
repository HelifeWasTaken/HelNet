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
#include "HelNet"

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        spdlog::error("Usage: {} <host> <tcp-port>", argv[0]);
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
#include "HelNet"

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
        spdlog::error("Usage: {} <host> <port> [protocol(tcp*/udp)]", argv[0]);
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
        spdlog::error("Unknown protocol: {}", protocol);
        return 1;
    }
}
```
