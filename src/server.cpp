#include "server.h"

#define PORT "12345"

class Info {
public:
        ClientInfo* client_info;
        pqxx::connection* C;
        Info(ClientInfo* _client_info, pqxx::connection* _C): client_info(_client_info), C(_C) {}
};

void * process_request(void * _info) {
	Info* info = (Info *)_info;

	char buf[10000] = { 0 };
    	int len = recv(info->client_info->fd, buf, sizeof(buf), 0);
    	if (len > 0) {
		std::string req(buf);
		std::string resp = execute_request(req, info->C);
		send(info->client_info->fd, resp.c_str(), resp.size(), 0);
	}
	return nullptr;
}

int main(){
    Server server(PORT);
    pqxx::connection *C = start_connection();
    
    while (1) {
        ClientInfo* client_info = server.accept_connection();
       	Info* info = new Info(client_info, C);	
	pthread_t thread;
	pthread_create(&thread, NULL, process_request, (void *)info);
    }

    return EXIT_SUCCESS;
}
