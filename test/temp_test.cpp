#include "client.h"
#include <sstream>
#include <string>

int main() {
	Client client("127.0.0.1", "12345");
  std::string req = "<transactions id=\"2\"><query id=\"1937\"/></transactions>";
  std::cout << "request:\n" << req << "\n";
  unsigned size = req.size();
  send(client.socket_fd, (char*)&size, sizeof(unsigned),0);
  send(client.socket_fd, req.c_str(), req.size(), 0);
    unsigned xml_len = 0;
    char buf[10000]{0};
    recv(client.socket_fd, (char *)&xml_len, sizeof(unsigned), MSG_WAITALL);
    recv(client.socket_fd, buf, sizeof(buf), MSG_WAITALL); 
    std::cout << "xml_len = " << xml_len << std::endl; 
    std::cout << buf << std::endl;
}
