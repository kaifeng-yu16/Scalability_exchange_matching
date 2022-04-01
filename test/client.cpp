#include "client.h"

class ProxyConnExc : public std::exception {
public:
  ProxyConnExc(std::string msg) {
    err_msg = msg;
  }
  virtual const char * what() const throw() {
    return err_msg.c_str();
  }
private:
  std::string err_msg;
};


void Client::start() {
    memset(&host_info, 0, sizeof(host_info));

    host_info.ai_family   = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if (status != 0) {
        //std::cout << "ERROR cannot get address info for host (" << hostname  << ", " << port << ")\n";
        throw ProxyConnExc("proxy conn err");
    }

    socket_fd = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol);
    if (socket_fd == -1) {
        //std::cout << "ERROR cannot create socket (" << hostname  << ", " << port << ")\n";
        throw ProxyConnExc("proxy conn err");
    }

    status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        //std::cout << "ERROR cannot connect to socket (" << hostname  << ", " << port << ")\n";
        throw ProxyConnExc("proxy conn err");
    }
}

int Client::recieve(std::vector<char> * buf) {
    int buf_size = buf->size() - 1;
    char * buf_ptr = buf->data();
    int len = 0;
    do {
      len = recv(socket_fd, buf_ptr, sizeof(char) * buf_size, 0);
      buf_size -= len;
      buf_ptr += len;
    } while (len > 0  && buf_size > 0);
    return buf->size() - 1 - buf_size; 
}
