#ifndef _SOCKET_H_
#define _SOCKET_H_

#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>

class Socket {
public:
    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo * host_info_list;
    const char * hostname;
    const char * port;

    Socket(const char * _port) : host_info_list(NULL), hostname(NULL), port(_port) {}
    Socket(const char * _hostname, const char * _port) : host_info_list(NULL), hostname(_hostname), port(_port) {}
    ~Socket(){
        close(socket_fd);
        freeaddrinfo(host_info_list);
    }
};

#endif
