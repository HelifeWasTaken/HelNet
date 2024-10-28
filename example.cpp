#define HL_NET_LOG_LEVEL HL_NET_LOG_LEVEL_MIN
#define HL_NET_AUTO_SETUP_LOG_SERVICE

#include <iostream>
#include "HelNet.hpp"

static void client_handle_on_receive(hl::net::client_t client,
                              hl::net::shared_buffer_t buffer,
                              const size_t& size)
{
    try {
        std::string str(reinterpret_cast<char*>(buffer->data()), size);
        HL_NET_LOG_CRITICAL("Received: {}, from client: {}", str, client->get_alias());
    } catch (const std::exception& e) {
        HL_NET_LOG_CRITICAL("Exception when printing: {}", e.what());
    }
}

template<class Protocol>
void client_routine(const std::string& host, const std::string& port)
{
    Protocol client;

    if (client.connect(host, port) == false)
    {
        return;
    }

    // Write all received data to stdout asynchronously
    client.callbacks_register().set_on_receive(std::bind(client_handle_on_receive, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    client.callbacks_register().set_on_receive_async(true);

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

static void server_handle_on_receive(hl::net::server_t server,
                              hl::net::connection_t client,
                              hl::net::shared_buffer_t buffer,
                              const size_t& size)
{
    try {
        std::string str(reinterpret_cast<char*>(buffer->data()), size);
        if (str == "exit" || str == "exit\n" || str == "exit\r\n") { // Handle nc and telnet style
            HL_NET_LOG_CRITICAL("Received: exit - closing server...");
            server->request_stop(); // Marks the server as unhealthy (will close on next iteration)
            return;
        }
        HL_NET_LOG_CRITICAL("Received: {}, echoing back to client {}", str, client->get_id());
        client->send(buffer, size);
    } catch (const std::exception& e) {
        HL_NET_LOG_CRITICAL("Exception when printing: {} - closing server...", e.what());
        server->stop();
    }
}

template<class Protocol>
void server_routine(const std::string& port)
{
    Protocol server;

    if (server.start(port) == false) {
        SPDLOG_CRITICAL("Failed to start server on port {}", port);
        return;
    }

    server.callbacks_register().set_on_receive(std::bind(server_handle_on_receive, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
    server.callbacks_register().set_on_receive_async(true);

    HL_NET_IF_CONSTEXPR (std::is_same<Protocol, hl::net::udp_server>::value) {
        //TODO
 //       hl::net::plugins::server_clients_timeout timeout_plugin(2000); // 2000ms
//        hl::net::plugins::server_clients_timeout::attach(server, timeout_plugin);
        while (server) {
            std::this_thread::sleep_for(std::chrono::seconds(1)); 
            // timeout_plugin.on_update(server); // TODO: Should be called automatically by the server
        }
    } else {
        while (server) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
    /*
    // Should become
    if constexpr(std::is_same<Protocol, hl::net::udp_server>::value) {
        server.attach_plugin<hl::net::plugins::server_clients_timeout>(server, 2000); // 2000ms
    }
    while (server) { 
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    */
    HL_NET_LOG_CRITICAL("Server closed");
}

int main(int argc, char **argv)
{
    HL_NET_SETUP_LOG_LEVEL();

    std::string port, mode = "connect", protocol = "tcp", host = "0.0.0.0";

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
        std::cout << argv[0] << ": <port> [*connect|listen] [*tcp|udp] [host = 0.0.0.0 (ignored for listen)]" << std::endl;
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
            server_routine<hl::net::udp_server>(port);
        }
    } else if (mode == "connect") {
        protocol == "tcp" ? client_routine<hl::net::tcp_client>(host, port) : client_routine<hl::net::udp_client>(host, port);
    } else {
        std::cout << "Unsupported mode (only connect/listen)" << std::endl;
    }
    return 0;
}
