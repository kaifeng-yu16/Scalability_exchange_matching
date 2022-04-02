#include "server.h"

#define PORT "12345"

int count = 0;
static std::mutex mtx;

class Info {
public:
        ClientInfo* client_info;
        pqxx::connection* C;
        struct timespec start;
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
  if (len <= 0 || xml_len == 0) {
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
    //////
    /*
    for (int m = 0; m < 10000; ++m) {
      int * a = new int(0);
      delete a;
    }*/
    /////
	  std::string resp = execute_request(req, C);
    C->disconnect();
    unsigned size = resp.size();
    send(info->client_info->fd, (char*)&size, sizeof(unsigned), MSG_NOSIGNAL);
    if (size != 0) {
      send(info->client_info->fd, resp.c_str(), resp.size(), MSG_NOSIGNAL); 
    }
    struct timespec  end;
    clock_gettime(CLOCK_REALTIME, &end);
    double diff = (1000000000.0 *(end.tv_sec - info->start.tv_sec) + end.tv_nsec - info->start.tv_nsec) / 1e9;
    //std::cout << diff << " resp: " << resp<< "\n";
    std::cout << diff << "\n";
    /*
    mtx.lock();
    ++count;
    mtx.unlock();*/
  }
  close(info->client_info->fd);
  delete info;
	return nullptr;
}

int main(int argc, char ** argv){
    if (argc != 2 && argc != 3) {
      std::cout << "./server numofrequest\n";
      return -1;
    }
    unsigned num_of_req;
    try {
      num_of_req = std::strtoul(argv[1], nullptr, 10);
    }
    catch (const std::exception &e) {
      std::cout << "Invalid input\n";
      std::cout << "./client host_name thread_num \n";
      return -1;
    }
    bool init_server = true;
    if (argc == 3) {
      init_server = false;
    }
    Server server(PORT);
    pqxx::connection *C = start_connection();
    if (init_server) {
      create_table(C);
    }
    end_connection(C);
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
    while (1) {
      /*
      if (num_of_req) {
        mtx.lock();
        if (count > num_of_req) {
          mtx.unlock();
          break;
        }
        mtx.unlock();
      }*/
        ClientInfo* client_info = server.accept_connection();
       	Info* info = new Info(client_info, C);	
	      ////
        info->start=start;
        //clock_gettime(CLOCK_REALTIME, &end);
        //double diff = (1000000000.0 *(end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec) / 1e9;
        //std::cout << diff << "\n";
        ////
        pthread_t thread;
        pthread_create(&thread, NULL, process_request, (void *)info);
        //std::cout << diff << "\n";

    }
    clock_gettime(CLOCK_REALTIME, &end);
    double diff = (1000000000.0 *(end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec) / 1e9;
    std::cout << "Ececution time: " << diff << " s\n";
    return EXIT_SUCCESS;
}
