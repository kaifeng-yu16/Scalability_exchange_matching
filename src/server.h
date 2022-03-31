#ifndef SERVER__H
#define SERVER__H

#include <vector>
#include "socket.h"
#include "request.h"

class ClientInfo {
public:
    int fd;
    int port;
    std::string ip_addr;
};

class Server : public Socket {
public:
    Server(const char * _port) : Socket(_port) { start(); }
    void start();
    ClientInfo * accept_connection();
};

void Server::start() {
    memset(&host_info, 0, sizeof(host_info));

    host_info.ai_family   = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags    = AI_PASSIVE;

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

    int yes = 1;
    status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        std::cout << "ERROR cannot bind socket (" << hostname  << ", " << port << ")\n";
        exit(EXIT_FAILURE);
    }

    status = listen(socket_fd, 100);
    if (status == -1) {
        std::cout << "ERROR cannot listen on socket (" << hostname  << ", " << port << ")\n";
        exit(EXIT_FAILURE);
    }
}

ClientInfo * Server::accept_connection(){
    struct sockaddr_storage socket_addr;
    socklen_t socket_addr_len = sizeof(socket_addr);
    
    int client_connection_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
    if (client_connection_fd == -1) {
        std::cout << "ERROR cannot accept connection on socket";
        exit(EXIT_FAILURE);
    }
    
    ClientInfo * client_info = new ClientInfo();
    client_info->fd = client_connection_fd;
    client_info->ip_addr = inet_ntoa(((struct sockaddr_in *)&socket_addr)->sin_addr);
    return client_info;
}

#endif
