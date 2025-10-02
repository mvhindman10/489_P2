#include <iostream>
#include <string>
#include <vector>
#include <spdlog/spdlog.h>
#include <cxxopts.hpp>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    // Argument parsing
    cxxopts::Options options("loadBalancer", "Video CDN Load Balancer");
    options.add_options()
        ("p,port", "Port to listen on", cxxopts::value<int>())
        ("g,geo", "Geographic mode (mutually exclusive with --rr)")
        ("r,rr", "Round robin mode (mutually exclusive with --geo)")
        ("s,servers", "Path to server info file", cxxopts::value<std::string>())
        ("h,help", "Print usage");

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }

    // required args
    if (!result.count("port") || !result.count("servers") ||
        (result.count("geo") == result.count("rr"))) {
        std::cerr << "Error: Must specify --port, --servers, and exactly one of --geo or --rr." << std::endl;
        std::cout << options.help() << std::endl;
        return 1;
    }
    int port = result["port"].as<int>();
    std::string serversFile = result["servers"].as<std::string>();
    bool geoMode = result.count("geo");
    bool rrMode = result.count("rr");

    if (port < 1024 || port > 65535) {
        std::cerr << "Error: Port must be in range [1024,65535]" << std::endl;
        return 1;
    }

    spdlog::info("Load balancer started on port {}", port);

    // IMPLEMENT: parse the server info file

    // set up listening socket
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        spdlog::error("Failed to create socket");
        return 1;
    }
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        spdlog::error("Failed to bind socket to port {}", port);
        close(listen_fd);
        return 1;
    }

    if (listen(listen_fd, 10) < 0) {
        spdlog::error("Failed to listen on socket");
        close(listen_fd);
        return 1;
    }

    // IMPLEMENT: main accept() loop
    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int conn_fd = accept(listen_fd, (struct sockaddr*)&client_addr, &client_len);
        if (conn_fd < 0) {
            spdlog::error("Failed to accept connection");
            continue;
        }
        // For now, just close the connection immediately
        close(conn_fd);
    }

    close(listen_fd);
    return 0;
}