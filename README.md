# HelNet

Multithreaded utilities using boost::asio and modern C++ for client/server apps (TCP &amp; UDP)

## Example of TCP Client

```cpp
#include "HelNet"

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cout << "Usage: " << argv[0] << " <ip> <port>" << std::endl;
        return 1;
    }

    hl::net::base_tcp_client client;

    client.set_on_receive([](hl::net::base_abstract_client&, const hl::net::buffer_t &buffer, const size_t &bytes_transferred) {
        std::cout << "Received " << bytes_transferred << " bytes" << std::endl;
        std::cout << std::string(buffer.begin(), buffer.begin() + bytes_transferred) << std::endl;
        // assuming the server sends a string
    });

    // Implicitly means that send is always authorized whatever the buffer content is
    // This is also the default behavior but provided for demonstration purposes
    client.set_on_send([](hl::net::base_abstract_client&, const hl::net::buffer_t &) {
        return true;
    });

    client.connect(argv[1], argv[2]);
}
```
