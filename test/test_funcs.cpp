#include "test_funcs.h"

std::atomic<bool> stop_signal = false;
static std::mutex mtx;

void send_transactions() {
  int count = 0;
  while (1) {
    ++ count;
    if (stop_signal) {
      mtx.lock();
      std::cout << "Transaction requests: " << count << std::endl;
      mtx.unlock();
      return;
    }
  }
}
