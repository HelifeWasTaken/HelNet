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
#include "HelNet.hpp"

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

## Usable client callbacks for TCP and UDP

```cpp
namespace hl
{
namespace net
{

    // Forward declaration
    class base_abstract_client_unwrapped;

    // Callbacks 
    // All callbacks are called using a separate thread except if stated otherwise

    using client_on_connect_callback            = std::function<void(base_abstract_client_unwrapped& client)>;

    // Is not called using a separate thread
    using client_on_connect_error_callback      = std::function<void(base_abstract_client_unwrapped& client, const boost::system::error_code& ec)>;

    using client_on_disconnect_callback         = std::function<void(base_abstract_client_unwrapped& client)>;

    // on_receive_* sends a copy of the buffer
    using client_on_receive_callback            = std::function<void(base_abstract_client_unwrapped& client, buffer_t& buffer_copy, const size_t& recv_bytes)>;
    using client_on_receive_error_callback      = std::function<void(base_abstract_client_unwrapped& client, buffer_t& buffer_copy, const boost::system::error_code& ec, const size_t& recv_bytes)>;

    /*
    The buffer used in on_send_* is the same buffer that is sent
    This means that the buffer is not copied but sent as a reference
    Please keep this in mind
    */

    // If client_on_send_callback returns false the buffer will not be sent
    // This callback is not called when the buffer is empty
    // Is not called using a separate thread
    // on_send can be used to implement compression or encryption or validation of the buffer (and more)
    using client_on_send_callback               = std::function<bool(base_abstract_client_unwrapped& client, buffer_t& buffer_copy, size_t& size)>;

    using client_on_send_error_callback         = std::function<void(base_abstract_client_unwrapped& client, const buffer_t& buffer_copy, const boost::system::error_code& ec, const size_t& sent_bytes)>;
    using client_on_send_success_callback       = std::function<void(base_abstract_client_unwrapped& client, const buffer_t& buffer_copy, const size_t& sent_bytes)>;


    class base_abstract_client_unwrapped
    {
    public:
        void set_on_connect(const client_on_connect_callback& callback);
        void set_on_connect_error(const client_on_connect_error_callback& callback);
        void set_on_disconnect(const client_on_disconnect_callback& callback);
        void set_on_receive(const client_on_receive_callback& callback);
        void set_on_receive_error(const client_on_receive_error_callback& callback);
        void set_on_send(const client_on_send_callback& callback);
        void set_on_send_error(const client_on_send_error_callback& callback);
        void set_on_send_success(const client_on_send_success_callback& callback);

        /*
        ...
        */
    };
}
}
```

### Down there is pseudo code until i figure a proper compression example

Example of a hello world tcp client but all the data is sent compressed

```cpp

// Tcp Client and Udp client already have all the callbacks implemented by default
// But you can override them if you want to

#include "HelNet.hpp"

// function that compress the data and replace the values by reference
bool compress_data(hl::net::buffer_t& data, size_t& byte_count)
{
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        spdlog::error("Usage: {} <host> <tcp-port>", argv[0]);
        return 1;
    }

    hl::net::tcp_client client(argv[1], argv[2]);

    client.set_on_connect([](hl::net::base_abstract_client_unwrapped& client) {
        spdlog::info("Connected to server");
    });

    client.set_on_send([](hl::net::base_abstract_client_unwrapped& client, hl::net::buffer_t& buffer, size_t& size) {
        return compress_data(buffer, size);
    });

    while (client.healthy()) {
        client.send_string("hello world");
    }
    return 0;
}
```
