#include "test_funcs.h"
#include "../src/request.h"
#include <thread>
#include <vector>
#include <time.h>
#include <atomic>
#include <cstdlib>
#include <pqxx/pqxx>

extern std::atomic<bool> stop_signal;
extern int count;

// ./client host_name thread_num exec_time
int main(int argc, char ** argv) {
  if (argc != 3) {
    std::cout << "./client host_name thread_num\n";
    return -1;
  }
  const char * host_name = argv[1];
  unsigned long thread_num;
  try {
    thread_num = std::strtoul(argv[2], nullptr, 10);
  }
  catch (const std::exception &e) {
    std::cout << "Invalid input\n";
    std::cout << "./client host_name thread_num \n";
    return -1;
  }
  
  size_t account_num = 1000;
  size_t sym_num = 50;
  
  std::vector<std::thread> threads;
  for (size_t i = 0 ; i < thread_num; ++i) {
    threads.push_back(std::thread(send_transactions, host_name, account_num, sym_num, 2, 10)); 
  }
  
  //std::thread t(send_create, host_name, account_num, sym_num);

  // start timing
  //struct timespec start, end;
  //clock_gettime(CLOCK_REALTIME, &start);
  /*
  while (1) {
    clock_gettime(CLOCK_REALTIME, &end);
    uint64_t diff = (1000000000.0 *(end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec) / 1e9;
    if (diff > exec_time) {
      std::cout << "Excetue " << count << "requests.\n";
      exit(EXIT_FAILURE);
      stop_signal = true;
      for (auto &t:threads) {
        t.join();
	stop_signal = true;
      }
      return 0;
    }
  }*/
  for (auto &t:threads) {
    t.join();
  }
  return 0;

}

