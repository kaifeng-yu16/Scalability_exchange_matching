#ifndef TEST_FUNCS__H
#define TEST_FUNCS__H

#include "client.h"
#include <iostream>
#include <atomic>
#include <mutex>
#include <sstream>
#include <string>
#include <ctime>
#include <cstdlib>
#include <pqxx/pqxx>
#include "../lib/rapidxml_ext.hpp"

// generate sufficient data to start
void send_transactions(const char * host_name, size_t account_num, size_t sym_num);
void initialize_data(const char * host_name, size_t account_num, size_t sym_num, size_t order_num);

#endif
