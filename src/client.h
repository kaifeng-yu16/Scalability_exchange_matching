#include "socket.h"

class Client : public Socket {
public:
    Client(const char * _hostname, const char * _port) : Socket(_hostname, _port) { start(); }
    void start();
};

void Client::start() {
    memset(&host_info, 0, sizeof(host_info));

    host_info.ai_family   = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
        std::cout << "ERROR cannot get address info for host (" << hostname  << ", " << port << ")\n";
        exit(EXIT_FAILURE);
    }

    socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol);
    if (socket_fd == -1) {
        std::cout << "ERROR cannot create socket (" << hostname  << ", " << port << ")\n";
        exit(EXIT_FAILURE);
    }

    status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        std::cout << "ERROR cannot connect to socket (" << hostname  << ", " << port << ")\n";
        exit(EXIT_FAILURE);
    }
}
