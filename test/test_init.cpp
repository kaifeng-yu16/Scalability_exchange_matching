#include "test_funcs.h"
#include "../src/request.h"
#include <thread>
#include <vector>
#include <time.h>
#include <atomic>
#include <cstdlib>
#include <pqxx/pqxx>

int main(int argc, char ** argv) {
  if (argc != 2) {
    std::cout << "./client host_name\n";
    return -1;
  }
  const char * host_name = argv[1];
  // parameters
 /* 
  size_t account_num = 10;
  size_t sym_num = 2;
  size_t order_num = 100;
  */
  
  size_t account_num = 50;
  size_t sym_num = 5;
  size_t order_num = 100;
 
  // pre created a bunch of account and sym and order
  
  initialize_data(host_name, account_num, sym_num, order_num);
  std::cout << "Finished initializing data.\n";
  return 0;
}

