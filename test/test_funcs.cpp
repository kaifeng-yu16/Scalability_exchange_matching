#include "test_funcs.h"

std::atomic<bool> stop_signal = false;
static std::mutex mtx;

void send_transactions(const char * host_name) {
  int count = 0;
  while (!stop_signal) {
    Client client(host_name, "12345");
	  std::string req = "<create><account id=\"1\" balance=\"2000\"/><account id=\"3\" balance=\"1000\"/><symbol sym=\"SYM\"><account id=\"1\">20</account><account id=\"1\">20</account><account id=\"2\">20</account><account id=\"3\">20</account></symbol><symbol sym=\"SYM\"><account id=\"1\">20</account><account id=\"2\">20</account></symbol></create>";
	  unsigned size = req.size();
    send(client.socket_fd, (char*)&size, sizeof(unsigned),0);
    send(client.socket_fd, req.c_str(), req.size(), 0);
    char buf[100000] {0};
    recv(client.socket_fd, buf, sizeof(buf), MSG_WAITALL);
    close(client.socket_fd);
    //std::cout << buf;
    ++ count;
  }
  mtx.lock();
  std::cout << "Transaction requests: " << count << std::endl;
  mtx.unlock();
  return;
}
