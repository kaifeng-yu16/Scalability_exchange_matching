#include "client.h"

int main() {
	Client client("127.0.0.1", "12345");
	std::string req = "<create><account id=\"1\" balance=\"2000\"/><symbol sym=\"SYM\"><account id=\"1\">20</account></symbol></create>";
	send(client.socket_fd, req.c_str(), req.size(), 0);
}
