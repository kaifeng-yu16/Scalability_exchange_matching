#include "server.h"

#define PORT "12345"

class Info {
public:
        ClientInfo* client_info;
        pqxx::connection* C;
        Info(ClientInfo* _client_info, pqxx::connection* _C): client_info(_client_info), C(_C) {}
        ~Info() {
          if (client_info != nullptr) {
            delete client_info;
          }
        }
};

void * process_request(void * _info) {
	Info* info = (Info *)_info;
  unsigned xml_len = 0;
  int len = recv(info->client_info->fd, (char *)&xml_len, sizeof(unsigned), MSG_WAITALL);
  // connection failed OR invalid request
  if (len == 0 || xml_len == 0) {
    close(info->client_info->fd);
    delete info;
    return nullptr;
  }
	std::vector<char> buf(xml_len + 1, 0);
  int buf_size = xml_len;
  char * buf_ptr = buf.data();
  do {
    len = recv(info->client_info->fd, buf_ptr, sizeof(char) * buf_size, 0);
    buf_size -= len;
    buf_ptr += len;
   } while (len > 0  && buf_size > 0);
  if (buf_size < xml_len) {
		std::string req(buf.data());
    pqxx::connection *C = nullptr;
    while(C == nullptr) {
      C = start_connection();
    }
		std::string resp = execute_request(req, C);
		send(info->client_info->fd, resp.c_str(), resp.size(), 0);
  
    C->disconnect();
	}
  close(info->client_info->fd);
  delete info;
	return nullptr;
}

int main(){
    Server server(PORT);
    pqxx::connection *C = start_connection();
    create_table(C);
    end_connection(C);
    while (1) {
        ClientInfo* client_info = server.accept_connection();
       	Info* info = new Info(client_info, C);	
	      pthread_t thread;
	      pthread_create(&thread, NULL, process_request, (void *)info);
    }
    end_connection(C);
    return EXIT_SUCCESS;
}
