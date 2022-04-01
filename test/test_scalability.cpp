#include "test_funcs.h"
#include "../src/request.h"
#include <thread>
#include <vector>
#include <time.h>
#include <atomic>
#include <cstdlib>
#include <pqxx/pqxx>

extern std::atomic<bool> stop_signal;

// ./client host_name thread_num exec_time
int main(int argc, char ** argv) {
  if (argc != 4) {
    std::cout << "./client host_name thread_num exec_time\n";
    return -1;
  }
  const char * host_name = argv[1];
<<<<<<< HEAD
  unsigned long thread_num, exec_time;
=======
  // unsigned long thread_num, exec_time;
  unsigned long exec_time;
>>>>>>> 7115f22c57d3b5cef363a1b21d64ad5c8e011ba9
  try {
    // thread_num = std::strtoul(argv[2], nullptr, 10);
    exec_time = std::strtoul(argv[3], nullptr, 10);
  }
  catch (const std::exception &e) {
    std::cout << "Invalid input\n";
    std::cout << "./client host_name thread_num exec_time\n";
    return -1;
  }
  // parameters
  
  size_t account_num = 10;
  size_t sym_num = 2;
  size_t order_num = 500;
  
  /*
  size_t account_num = 1000;
  size_t sym_num = 50;
  size_t order_num = 2000;
  */
  // pre created a bunch of account and sym and order
  
  initialize_data(host_name, account_num, sym_num, order_num);
<<<<<<< HEAD
  std::cout << "Finished initializing data.\n";
=======
  
  /*
>>>>>>> 7115f22c57d3b5cef363a1b21d64ad5c8e011ba9
  std::vector<std::thread> threads;
  for (size_t i = 0 ; i < thread_num; ++i) {
    threads.push_back(std::thread(send_transactions, host_name, account_num, sym_num,2, 10));
  }
  */
  std::thread t(send_create, host_name, account_num, sym_num);

  // start timing
  struct timespec start, end;
  clock_gettime(CLOCK_REALTIME, &start);
  while (1) {
    clock_gettime(CLOCK_REALTIME, &end);
    uint64_t diff = (1000000000.0 *(end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec) / 1e9;
    if (diff > exec_time) {
      stop_signal = true;
      /* 
      for (auto &t:threads){
        t.join();
      }
      */
      t.join();
      return 0;
    }
  }
}

