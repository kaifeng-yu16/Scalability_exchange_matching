#ifndef _CLIENT_H_
#define _CLIENT_H_

#include "socket.h"
#include <vector>

class Client : public Socket {
public:
    Client(const char * _hostname, const char * _port) : Socket(_hostname, _port) { start(); }
    void start();
    int recieve(std::vector<char> * buf);
};

#endif
