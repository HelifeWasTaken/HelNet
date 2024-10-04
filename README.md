# HelNet

Multithreaded utilities using boost::asio and modern C++ for client/server apps (TCP &amp; UDP)

## Example of TCP Client

```cpp
#include "HelNet"

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cout << "Usage: " << argv[0] << " <ip> <tcp-port>" << std::endl;
        return 1;
    }

    auto client = hl::net::tcp_client::make();

    client->set_on_receive([](hl::net::base_abstract_client&, const hl::net::buffer_t &buffer, const size_t &bytes_transferred) {
        std::cout << "Received " << bytes_transferred << " bytes" << std::endl;
        std::cout << "Buffer content: " << std::string(buffer.begin(), buffer.end()) << std::endl;
    });
    client->set_on_send([](hl::net::base_abstract_client&, const hl::net::buffer_t &) { return true; });

    while (true);
}
```

## Example of UDP Client

```cpp
#include "HelNet"

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cout << "Usage: " << argv[0] << " <ip> <tcp-port>" << std::endl;
        return 1;
    }

    auto client = hl::net::udp_client::make();

    client->set_on_receive([](hl::net::base_abstract_client&, const hl::net::buffer_t &buffer, const size_t &bytes_transferred) {
        std::cout << "Received " << bytes_transferred << " bytes" << std::endl;
        std::cout << "Buffer content: " << std::string(buffer.begin(), buffer.end()) << std::endl;
    });
    client->set_on_send([](hl::net::base_abstract_client&, const hl::net::buffer_t &) { return true; });

    while (true);
}
```
