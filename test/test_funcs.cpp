#include "test_funcs.h"

//std::atomic<bool> stop_signal = false;
//static std::mutex mtx;
//int count=0;


// Insert records to table ACCOUNT and SYMBOL.
void initialize_data(const char * host_name, size_t account_num, size_t sym_num, size_t order_num) {
  for (size_t i = 0; i < account_num; ++i) {
    Client client(host_name, "12345");
    std::stringstream ss;
    ss << "<create><account id=\"" << std::to_string(i) << "\" balance=\"100000\"/></create>";
    std::string req = ss.str();
    //std::cout << "request:\n" << req << "\n";
    unsigned size = req.size();
    send(client.socket_fd, (char*)&size, sizeof(unsigned),0);
    send(client.socket_fd, req.c_str(), req.size(), 0);
    
    unsigned xml_len = 0;
    char buf[10000]{0};
    recv(client.socket_fd, (char *)&xml_len, sizeof(unsigned), MSG_WAITALL);
    recv(client.socket_fd, buf, sizeof(buf), MSG_WAITALL); 
    //std::cout << buf << std::endl;
    
    close(client.socket_fd);
  }
  
  for (size_t i = 0; i < sym_num; ++i) {
    Client client(host_name, "12345");
    std::stringstream ss;
    ss << "<create><symbol sym=\"" << std::to_string(i) << "\">";
    for (size_t j = 0; j < account_num; ++j) {
      ss << "<account id=\"" << std::to_string(j) << "\">1000</account>";
    }
    ss << "</symbol></create>";
    std::string req = ss.str();
    //std::cout << "request:\n" << req << "\n";
    unsigned size = req.size();
    send(client.socket_fd, (char*)&size, sizeof(unsigned),0);
    send(client.socket_fd, req.c_str(), req.size(), 0);
    
    unsigned xml_len = 0;
    char buf[100000]{0};
    recv(client.socket_fd, (char *)&xml_len, sizeof(unsigned), MSG_WAITALL);
    recv(client.socket_fd, buf, sizeof(buf), MSG_WAITALL); 
    //std::cout << buf << std::endl;
    
    close(client.socket_fd);
  }
  srand ((unsigned int)time(NULL));
  int limit_low = 50;
  int limit_high = 200;
  int amount_low = -100;
  int amount_high = 100;
  for (size_t i = 0; i < order_num; ++i) {
    Client client(host_name, "12345");
    std::stringstream ss;
    std::string id = std::to_string(rand() % account_num);
    std::string sym = std::to_string(rand() % sym_num);
    std::string lim = std::to_string(rand() % (limit_high - limit_low) + limit_low);
    std::string amt = std::to_string(rand() % (amount_high - amount_low) + amount_low);
    ss << "<transactions id=\"" << id << "\"><order sym=\"" << sym << "\" amount=\"" << amt << "\" limit=\"" << lim << "\"/> </transactions>";
	  std::string req = ss.str();
    //std::cout << "request:\n" << req << "\n";
    unsigned size = req.size();
    send(client.socket_fd, (char*)&size, sizeof(unsigned),0);
    send(client.socket_fd, req.c_str(), req.size(), 0);
  
    unsigned xml_len = 0;
    char buf[100000]{0};
    recv(client.socket_fd, (char *)&xml_len, sizeof(unsigned), MSG_WAITALL);
    recv(client.socket_fd, buf, sizeof(buf), MSG_WAITALL); 
    //std::cout << buf << std::endl;
    
    close(client.socket_fd);
  }
}

void send_create(const char * host_name, size_t account_num, size_t sym_num) {
  srand ((unsigned int)time(NULL));
  //int count = 0;

  //while (!stop_signal && account_num > 0) {
  while (account_num > 0) {
    // Create new account.
    Client client(host_name, "12345");
    std::stringstream ss;
    ss << "<create><account id=\"" << std::to_string(++account_num) << "\" balance=\"100000\"/></create>";
    std::string req = ss.str();
    unsigned size = req.size();
    send(client.socket_fd, (char*)&size, sizeof(unsigned),0);
    send(client.socket_fd, req.c_str(), req.size(), 0);
    unsigned xml_len = 0;
    int len = recv(client.socket_fd, (char *)&xml_len, sizeof(unsigned), MSG_WAITALL);
    if (len == 0 || xml_len == 0) {
      close(client.socket_fd);
      /*if (len != 0) {
        mtx.lock();
	      ++count;
	      mtx.unlock();
      }*/
      continue;
    }
    std::vector<char> buf(xml_len + 1, 0);
    int buf_size = client.recieve(&buf);
    for (size_t i = 0; i < buf.size(); i ++) {
      std::cout << buf[i];
    }
    close(client.socket_fd);
    /*mtx.lock();
    ++ count;
    mtx.unlock();*/
    if (buf_size == 0) {
      continue;
    }

    // 50% create position for newly created account.
    if (rand() % 2 < 1) {
      Client client(host_name, "12345");
      std::stringstream ss;

      int r = rand();
      std::cout << "rand: " << r << std::endl; 
      ss << "<create><symbol sym=\"" << std::to_string(r % sym_num + 1) << "\">";
      ss << "<account id=\"" << std::to_string(account_num) << "\">1000</account>";
      ss << "</symbol></create>";
      std::string req = ss.str();
      std::cout << "request: " << req << std::endl;
      unsigned size = req.size();
      send(client.socket_fd, (char*)&size, sizeof(unsigned),0);
      send(client.socket_fd, req.c_str(), req.size(), 0);
      unsigned xml_len = 0;
      int len = recv(client.socket_fd, (char *)&xml_len, sizeof(unsigned), MSG_WAITALL);
      if (len == 0 || xml_len == 0) {
        close(client.socket_fd);
        /*if (len != 0) {
          mtx.lock();
	        ++count;
	        mtx.unlock();
        }*/
        continue;
      }
      std::vector<char> buf(xml_len + 1, 0);
      int buf_size = client.recieve(&buf);
      close(client.socket_fd);
      for (size_t i = 0; i < buf.size(); i ++) {
      	std::cout << buf[i];
      }
      /*
      mtx.lock();
      ++ count;
      mtx.unlock();
      */
      if (buf_size == 0) {
        continue;
      }
    }
  }
  mtx.lock();
  std::cout << "Create requests: " << count << std::endl;
  mtx.unlock();
  return;
}

// assume each account has 100000 balance, and has 1000 position for each sym
// account_num and sym_num should not exceed 32767, since the return value rand() is only guarenteed to be at least 32767
void send_transactions(const char * host_name, size_t account_num, size_t sym_num, size_t query_rate, size_t cancel_rate) {
  srand ((unsigned int)time(NULL));
  int limit_low = 50;
  int limit_high = 200;
  int amount_low = -100;
  int amount_high = 100;

  //int count = 0;

  while (1) {
   // std::cout << stop_signal << std::endl;
    try {
      Client client(host_name, "12345");
      std::stringstream ss;
      std::string id = std::to_string(rand() % account_num);
      std::string sym = std::to_string(rand() % sym_num);
      std::string lim = std::to_string(rand() % (limit_high - limit_low) + limit_low);
      std::string amt = std::to_string(rand() % (amount_high - amount_low) + amount_low);
      ss << "<transactions id=\"" << id << "\"><order sym=\"" << sym << "\" amount=\"" << amt << "\" limit=\"" << lim << "\"/> </transactions>";
	    std::string req = ss.str();
      // std::cout << "request:\n" << req << "\n";
      unsigned size = req.size();
      send(client.socket_fd, (char*)&size, sizeof(unsigned),0);
      send(client.socket_fd, req.c_str(), req.size(), 0);
      unsigned xml_len = 0;
      int len = recv(client.socket_fd, (char *)&xml_len, sizeof(unsigned), MSG_WAITALL);
      if (len == 0 || xml_len == 0) {
        close(client.socket_fd);
        /*
        if (len != 0) {
          mtx.lock();
	        ++count;
	        mtx.unlock();
        }*/
        continue;
      }
      std::vector<char> buf(xml_len + 1, 0);
      int buf_size = client.recieve(&buf);
      close(client.socket_fd);
      //std::cout << buf;
      /*
      mtx.lock();
      ++ count;
      mtx.unlock();
      */
      if (buf_size <= 0 || (query_rate == 0 && cancel_rate == 0)) {
        continue;
      }
      // std::cout << "reponse: \nxml_len= " << xml_len << std::endl<< "buffer size = " << buf_size << std::endl<< buf.data() << std::endl;
      rapidxml::xml_document<> doc;  
      try {
        doc.parse<0>(buf.data()); 
      }
      catch (const std::exception &e) {
        continue;
      }
      rapidxml::xml_node<>* root = doc.first_node();
      if (root == 0 || strcmp(root->name(), "results") != 0) {
        //std::cerr << "Wrong results from server!\n" << "buf_size=" << buf_size << " xml_len=" << xml_len << std::endl;
        continue;
      }
      rapidxml::xml_node<>* node = root->first_node("opened");
      if (node == 0) {
        // order not secessfuly opend
        continue;
      }
      rapidxml::xml_attribute<> *attr = node->first_attribute("id");
      if (attr == 0) {
        //std::cerr << "Wrong results from server: Opened order do not have an id.\n"; 
        continue;
      }
      std::string order_id(attr->value());
      // 10% chance to cancel order
      if (cancel_rate != 0 && rand() % cancel_rate < 1) {
        Client client(host_name, "12345");
        ss.str("");
        ss << "<transactions id=\"" << id << "\"><cancel id=\"" << order_id << "\"/></transactions>";
        std::string cancel_req = ss.str();
        //std::cout << "cancel_request:\n" << cancel_req << "\n";
        unsigned cancel_size = cancel_req.size();
        send(client.socket_fd, (char*)&cancel_size, sizeof(unsigned),0);
        send(client.socket_fd, cancel_req.c_str(), cancel_req.size(), 0);
        unsigned xml_len = 0;
        int len = recv(client.socket_fd, (char *)&xml_len, sizeof(unsigned), MSG_WAITALL);
        if (len == 0 || xml_len == 0) {
          close(client.socket_fd);
          /*if (len != 0) {
            mtx.lock();
	          ++count;
	          mtx.unlock();
          }*/
        } else {
	  std::vector<char> buf(xml_len + 1, 0);
          client.recieve(&buf);
          close(client.socket_fd);
          //std::cout << buf;
          /*
          mtx.lock();
	        ++ count;
	        mtx.unlock();*/
        }
      }
      // 50% chance to query order
      if (query_rate != 0 && rand() % query_rate < 1) {
        Client client(host_name, "12345");
        ss.str("");
        ss << "<transactions id=\"" << id << "\"><query id=\"" << order_id << "\"/></transactions>";
        std::string query_req = ss.str();
        //std::cout << "query_request:\n" << query_req << "\n";
        unsigned query_size = query_req.size();
        send(client.socket_fd, (char*)&query_size, sizeof(unsigned),0);
        send(client.socket_fd, query_req.c_str(), query_req.size(), 0);
        unsigned xml_len = 0;
        int len = recv(client.socket_fd, (char *)&xml_len, sizeof(unsigned), MSG_WAITALL);
        if (len == 0 || xml_len == 0) {
          close(client.socket_fd);
          /*
          if (len != 0) {
            mtx.lock();
	          ++count;
	          mtx.unlock();
          }*/
        } else {
	        std::vector<char> buf(xml_len + 1, 0);
          client.recieve(&buf);
          close(client.socket_fd);
          //std::cout << buf;
          /*
          mtx.lock();
	        ++ count;
	        mtx.unlock();*/
        }
      }
    }
    catch (std::exception &e) {
      //std::cerr << "Might have a proxy connection issue " << std::endl;
      continue;
    } 
  }
  /*mtx.lock();
  std::cout << "Transaction requests: " << count << std::endl;
  mtx.unlock();*/
  return;
}

