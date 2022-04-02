#include "server.h"

#define PORT "12345"

int count = 0;
static std::mutex mtx;

class Info {
public:
        ClientInfo* client_info;
        pqxx::connection* C;
        struct timespec start;
        bool load_test;
        Info(ClientInfo* _client_info, pqxx::connection* _C): client_info(_client_info), C(_C) {}
        ~Info() {
          if (client_info != nullptr) {
            delete client_info;
          }
        }
};

void time_consumer() {
  int a, b, c;
  for (int i = 0; i < 10000; ++i) {
    a = (b + i + i * i) * (c - 1 +i);
    c = (b + i + i * i) * (a - 1 +i);
    b = (b*b + c*c - i*i) * (a*a - i*i +i);
  }
}

void * process_request(void * _info) {
  /*for (int i = 0; i < 1000; ++i) {
    time_consumer();
  }*/
  //struct timespec  start;
  //clock_gettime(CLOCK_REALTIME, &start);
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
    if (info->load_test) {
      struct timespec  end;
      clock_gettime(CLOCK_REALTIME, &end);
      double diff = (1000000000.0 *(end.tv_sec - info->start.tv_sec) + end.tv_nsec - info->start.tv_nsec) / 1e9;
      //double diff = (1000000000.0 *(end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec) / 1e9;
      //std::cout << diff << " resp: " << resp<< "\n";
      std::cout << diff << "\n";
    }
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
    if (argc != 1 && argc != 2) {
      std::cout << "./server\n";
      std::cout << "To load test, run: ./server true\n";
      return -1;
    }
    bool load_test = false;
    if (argc == 2) {
      load_test= true;
    }
    Server server(PORT);
    pqxx::connection *C = start_connection();
    if (!load_test) {
      create_table(C);
    }
    end_connection(C);
    Threadpool thread_pool;
    Threadpool* pool = thread_pool.get_pool();
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
    while (1) {
        ClientInfo* client_info = server.accept_connection();
       	Info* info = new Info(client_info, C);	
        info->start=start;
        info->load_test = load_test;
        if (!pool->assign_task(bind(process_request, (void*)info))) {
          close(info->client_info->fd);
        }
        //pthread_t thread;
        //pthread_create(&thread, NULL, process_request, (void *)info);
        /*
        clock_gettime(CLOCK_REALTIME, &end);
        double diff = (1000000000.0 *(end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec) / 1e9;
        std::cout << "Ececution time: " << diff << " s\n";
        */
    }
    /*
    clock_gettime(CLOCK_REALTIME, &end);
    double diff = (1000000000.0 *(end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec) / 1e9;
    std::cout << "Ececution time: " << diff << " s\n";
    return EXIT_SUCCESS;
    */
}
