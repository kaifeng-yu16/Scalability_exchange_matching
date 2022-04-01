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
<<<<<<< HEAD
void send_transactions(const char * host_name, size_t account_num, size_t sym_num, size_t query_rate, size_t cancel_rate);
=======
void send_create(const char * host_name, size_t account_num, size_t sym_num);
void send_transactions(const char * host_name, size_t account_num, size_t sym_num);
>>>>>>> 7115f22c57d3b5cef363a1b21d64ad5c8e011ba9
void initialize_data(const char * host_name, size_t account_num, size_t sym_num, size_t order_num);

#endif
