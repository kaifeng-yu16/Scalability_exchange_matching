#include "test_funcs.h"
#include <thread>
#include <vector>
#include <time.h>
#include <atomic>

extern std::atomic<bool> stop_signal;

int main(int argc, char ** argv) {
  int thread_num = 10;
  std::vector<std::thread> threads;
  for (int i = 0 ; i < thread_num; ++i) {
    threads.push_back(std::thread(send_transactions));
  }
  
  // start timing
  struct timespec start, end;
  clock_gettime(CLOCK_REALTIME, &start);
  while (1) {
    clock_gettime(CLOCK_REALTIME, &end);
    uint64_t diff = (1000000000.0 *(end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec) / 1e9;
    if (diff > 10) {
      stop_signal = true;
      for (auto &t:threads){
        t.join();
      }
      return 0;
    }
  }
}

