#include "request.h"

std::string execute_request(std::string req, pqxx::connection* C) {
  try {
    std::string response = "";
    rapidxml::xml_document<> doc;  
    doc.parse<0>(const_cast<char*>(req.data())); 
    rapidxml::xml_node<>* root = doc.first_node();
    if (std::strcmp(root->name(), "create") == 0) {
      response = create_req(root, C);
    } else if (std::strcmp(root->name(), "transactions") == 0) {
      response = transaction_req(root, C);
    }
    if (response == "") {
      response = "<results>";
    }
    return response;
  }
  catch (const pqxx::pqxx_exception &e) {
    //std::cerr << "Database Error: " << e.base().what() << std::endl;
    return "<results>";
  }
  catch (std::exception &e) {
    //std::cerr << "Parser Error: " << e.what() << std::endl;
    return "<results>";
  }
}

void copy_attr(rapidxml::xml_node<>* node, rapidxml::xml_document<>* res_doc, rapidxml::xml_node<>* res_node) {
  for (rapidxml::xml_attribute<> *attr = node->first_attribute(); attr != 0; attr=attr->next_attribute()) {
    char *name = res_doc->allocate_string(attr->name()); 
    char *value = res_doc->allocate_string(attr->value()); 
    res_node->append_attribute(res_doc->allocate_attribute(name, value));
  }
}

// Add new account.
rapidxml::xml_node<>* add_new_account(rapidxml::xml_node<>* node, rapidxml::xml_document<>* res_doc, pqxx::connection* C) {
  // Generate results.
  rapidxml::xml_node<>* res_node = nullptr;

  //Get attribute id.
  rapidxml::xml_attribute<> *attr = node->first_attribute("id");
  if (attr == 0) {
    //std::cerr << "Create_Account: do not have id\n";
    res_node = res_doc->allocate_node(rapidxml::node_element, "error", "Do not have id attr");
    res_node->append_attribute(res_doc->allocate_attribute("id", ""));
    return res_node;
  }
  std::string id(attr->value());

  //Get attribute balance.
  attr = node->first_attribute("balance");
  if (attr == 0) {
    //std::cerr << "Create_Account: do not have balance\n";
    res_node = res_doc->allocate_node(rapidxml::node_element, "error", "Do not have balance attr");
    char * id_ptr = res_doc->allocate_string(id.c_str());
    res_node->append_attribute(res_doc->allocate_attribute("id", id_ptr));
    return res_node;
  }
  std::string balance(attr->value());

  // Insert new record into table.
  pqxx::work W(*C);
  try {
    std::stringstream ss;
    ss << "INSERT INTO ACCOUNT (ACCOUNT_ID, BALANCE) VALUES(" << W.quote(id) << ", " << W.quote(balance) << ");";
    pqxx::result R = W.exec(ss.str());
    W.commit();
    res_node = res_doc->allocate_node(rapidxml::node_element, "created");
    char * id_ptr = res_doc->allocate_string(id.c_str());
    res_node->append_attribute(res_doc->allocate_attribute("id", id_ptr));
  }
  catch (const pqxx::pqxx_exception & e) {
    W.abort();
    //std::cerr << "Database Error in <create> <account>: " << e.base().what() << std::endl;
    res_node = res_doc->allocate_node(rapidxml::node_element, "error", "Can not create net account, account might already exist OR has invalid account or balance info");
    char * id_ptr = res_doc->allocate_string(id.c_str());
    res_node->append_attribute(res_doc->allocate_attribute("id", id_ptr));
    return res_node;
  }
  return res_node;
}

// Add new position.
rapidxml::xml_node<>* add_new_position(std::string sym, rapidxml::xml_node<>* node, rapidxml::xml_document<>* res_doc, pqxx::connection* C) {
  // Generate results.
  rapidxml::xml_node<>* res_node = nullptr;

  // Get attribute id.
  rapidxml::xml_attribute<> *attr = node->first_attribute("id");
  if (attr == 0) {
    //std::cerr << "Create_Symbol: do not have id\n";
    res_node = res_doc->allocate_node(rapidxml::node_element, "error", "Do not have id attr");
    char * sym_ptr = res_doc->allocate_string(sym.c_str());
    res_node->append_attribute(res_doc->allocate_attribute("sym", sym_ptr));
    res_node->append_attribute(res_doc->allocate_attribute("id", ""));
    return res_node;
  }
  std::string id(attr->value());

  // Get value num.
  std::string num(node->value());
  
  if (num.size() == 0) {
    //std::cerr << "Create_Symbol: do not have num\n";
    res_node = res_doc->allocate_node(rapidxml::node_element, "error", "Do not have num");
    char * sym_ptr = res_doc->allocate_string(sym.c_str());
    res_node->append_attribute(res_doc->allocate_attribute("sym", sym_ptr));
    copy_attr(node, res_doc, res_node);
    return res_node;
  }
  int num_int; 
  try {
    num_int = stoi(num);
    if (num_int <= 0 ) {
      throw std::exception();
    }
  }
  catch (const std::exception &e){
    //std::cerr << "Do not have a valid NUM when creating a position.\n"; 
    res_node = res_doc->allocate_node(rapidxml::node_element, "error", "Do not have a valid NUM when creating a position.");
    char * sym_ptr = res_doc->allocate_string(sym.c_str());
    res_node->append_attribute(res_doc->allocate_attribute("sym", sym_ptr));
    copy_attr(node, res_doc, res_node);
    return res_node;
  }

  std::stringstream ss;
  // Insert new record into table.
  pqxx::work W(*C);
  try {
    // If sym is not stored in table SYMBOL, insert it into the table.
    ss << "INSERT INTO SYMBOL VALUES (" << W.quote(sym) << ") ON CONFLICT(NAME) DO NOTHING; ";
    ss << "INSERT INTO POSITION VALUES(" << W.quote(id) << ", " << W.quote(sym) << ", 0) ON CONFLICT(ACCOUNT_ID, SYMBOL) DO NOTHING;";
    ss << "UPDATE POSITION SET AMOUNT=AMOUNT+'" << num_int << "' WHERE ACCOUNT_ID=" << W.quote(id) << " AND SYMBOL=" << W.quote(sym) << ";";
    pqxx::result R = W.exec(ss.str());
    W.commit();
    res_node = res_doc->allocate_node(rapidxml::node_element, "created");
    char * sym_ptr = res_doc->allocate_string(sym.c_str());
    res_node->append_attribute(res_doc->allocate_attribute("sym", sym_ptr));
    copy_attr(node, res_doc, res_node);
  }
  catch (const pqxx::pqxx_exception & e) {
    W.abort();
    //std::cerr << "Database Error in <create> <symbol>: " << e.base().what() << std::endl;
    res_node = res_doc->allocate_node(rapidxml::node_element, "error", "Position can not be created by database, might have invalid input id.");
    char * sym_ptr = res_doc->allocate_string(sym.c_str());
    res_node->append_attribute(res_doc->allocate_attribute("sym", sym_ptr));
    copy_attr(node, res_doc, res_node);
    return res_node;
  }
  return res_node;
}

// Create request.
std::string create_req(rapidxml::xml_node<>* root, pqxx::connection* C) {
  // Generate response.
  rapidxml::xml_document<> res_doc;
  rapidxml::xml_node<>* res_root = res_doc.allocate_node(rapidxml::node_element, "results");
  res_doc.append_node(res_root);

  rapidxml::xml_node<>* node = root->first_node(); 
  if (node == 0) {
    //std::cerr << "Create: do not have any children node\n";
  }

  // Travle through all the child nodes.
  rapidxml::xml_node<>* res_child_node;
  while (node != 0) {
    // <account>
    if (std::strcmp(node->name(), "account") == 0) {
      res_child_node = add_new_account(node, &res_doc, C);
      res_root->append_node(res_child_node);
    } 
    // <symbol>
    else if (std::strcmp(node->name(), "symbol") == 0) {
      // Get attribute sym.
      rapidxml::xml_attribute<> *attr = node->first_attribute("sym");

      // Generate error message for every <account> under <symbol>.
      if (attr == 0) {
        //std::cerr << "Create_Symbol: do not have sym\n";
        for (rapidxml::xml_node<>* it = node->first_node("account"); it != 0; it = it->next_sibling("account")) {
          res_child_node = res_doc.allocate_node(rapidxml::node_element, "error", "Do not have sym attr");
          res_child_node->append_attribute(res_doc.allocate_attribute("sym", ""));
          copy_attr(it, &res_doc, res_child_node);
          res_root->append_node(res_child_node);
        }
      }
      else {
        std::string sym(attr->value());

        // <account>
        rapidxml::xml_node<>* child_node = node->first_node("account");
        if (child_node == 0) {
          //std::cerr << "Create_Symbol: do not have any child node\n";
          res_child_node = res_doc.allocate_node(rapidxml::node_element, "error", "Do not have any child node");
          res_child_node->append_attribute(res_doc.allocate_attribute("sym", sym.c_str()));
          res_root->append_node(res_child_node);
        }
        while (child_node != 0) {
          res_child_node = add_new_position(sym, child_node, &res_doc, C);
          res_root->append_node(res_child_node);
          child_node = child_node->next_sibling("account");
        }
      }
    }
    node = node->next_sibling();
  }

  std::string res;
  rapidxml::print(std::back_inserter(res), res_doc, 0);
  return res;
}

std::string transaction_req(rapidxml::xml_node<>* root, pqxx::connection* C) {
  rapidxml::xml_document<> res_doc;
  rapidxml::xml_node<>* res_root = res_doc.allocate_node(rapidxml::node_element, "results");
  res_doc.append_node(res_root);
  rapidxml::xml_attribute<> *attr = root->first_attribute("id");
  if (attr == 0) {
    //std::cerr << "Transaction: do not have user id\n";
    for (rapidxml::xml_node<>* node = root->first_node("order"); node != 0; node=node->next_sibling("order")) {
      rapidxml::xml_node<>* res_child_node = res_doc.allocate_node(rapidxml::node_element, "error", "Do not have user_id attr");
      res_root->append_node(res_child_node);
      copy_attr(node, &res_doc, res_child_node);
    }
  } else {
    std::string user_id(attr->value());
    pqxx::nontransaction N(*C);
    std::stringstream ss;
    ss << "SELECT * FROM ACCOUNT WHERE ACCOUNT_ID=" << N.quote(user_id) << ";";
    try {
      pqxx::result R(N.exec(ss.str()));
      N.commit();
      if (R.begin() == R.end()) {
        throw pqxx::argument_error("invalid user id");
      }
    }
    catch (const pqxx::pqxx_exception &e) {
      //std::cerr << "Transaction: invalid user id\n";
      for (rapidxml::xml_node<>* node = root->first_node("order"); node != 0; node=node->next_sibling("order")) {
        rapidxml::xml_node<>* res_child_node = res_doc.allocate_node(rapidxml::node_element, "error", "Invalid user_id attr");
        root->append_node(res_child_node);
        copy_attr(node, &res_doc, res_child_node);
      }
    }
    rapidxml::xml_node<>* node = root->first_node(); 
    if (node == 0) {
      //std::cerr << "Transaction: do not have any children node\n";
    }
    rapidxml::xml_node<>* res_child_node;
    while (node != 0) {
      if (std::strcmp(node->name(), "order") == 0) {
        res_child_node = add_new_order(user_id, node, &res_doc, C);
      } else if (std::strcmp(node->name(), "cancel") == 0) {
        res_child_node = cancel_order(user_id, node, &res_doc, C);
      } else if (std::strcmp(node->name(), "query") == 0) {
        res_child_node = query_order(user_id, node, &res_doc, C);
      } else {
        std::cout << "Transaction: have invalid child node[" << node->name() << "]\n";
        res_child_node = nullptr;
      }
      if (res_child_node != nullptr) {
        res_root->append_node(res_child_node);
      } 
      node = node->next_sibling();
    }
  }
  std::string res;
  rapidxml::print(std::back_inserter(res), res_doc, 0);
  return res;
}

rapidxml::xml_node<>* add_new_order(std::string user_id, rapidxml::xml_node<>* node, rapidxml::xml_document<>* res_doc, pqxx::connection* C) {
  rapidxml::xml_node<>* res_node = nullptr;
  rapidxml::xml_attribute<> *attr = node->first_attribute("sym");
  if (attr == 0) {
    //std::cerr << "Transaction_Order: do not have sym\n";
    res_node = res_doc->allocate_node(rapidxml::node_element, "error", "Do not have sym attr");
    copy_attr(node, res_doc, res_node);
    return res_node;
  }
  std::string sym(attr->value());
  attr = node->first_attribute("amount");
  if (attr == 0) {
    //std::cerr << "Transaction_Order: do not have amount\n";
    res_node = res_doc->allocate_node(rapidxml::node_element, "error", "Do not have amount attr");
    copy_attr(node, res_doc, res_node);
    return res_node;
  }
  std::string amount(attr->value());
  attr = node->first_attribute("limit");
  if (attr == 0) {
    //std::cerr << "Transaction_Order: do not have limit\n";
    res_node = res_doc->allocate_node(rapidxml::node_element, "error", "Do not have limit attr");
    copy_attr(node, res_doc, res_node);
    return res_node;
  }
  std::string limit(attr->value());
  pqxx::work W(*C);
  try {
    std::stringstream ss;
    if (stod(amount) > 0) { // buy order
      ss << "SELECT BALANCE FROM ACCOUNT WHERE ACCOUNT_ID=" << W.quote(user_id) << " AND BALANCE>='" << stod(limit) * stod(amount) << "' FOR UPDATE;"; 
      pqxx::result R(W.exec(ss.str()));
      if (R.begin() == R.end()) { // balance insufficient 
        W.abort();
        res_node = res_doc->allocate_node(rapidxml::node_element, "error", "insufficient balance to buy");
        copy_attr(node, res_doc, res_node);
        return res_node;
      }
      ss.str("");
      ss << "UPDATE ACCOUNT SET BALANCE=BALANCE-'" << stod(limit) * stod(amount) << "' WHERE ACCOUNT_ID=" << W.quote(user_id) << "; ";
      ss << "INSERT INTO SYMBOL VALUES (" << W.quote(sym) << ") ON CONFLICT(NAME) DO NOTHING; ";
      ss << "INSERT INTO SYM_ORDER(ACCOUNT_ID, STATUS, SYMBOL, AMOUNT, PRICE) VALUES (" << W.quote(user_id) << ", 'open', " << W.quote(sym) << ", " << W.quote(amount) << ", " << W.quote(limit) << ") RETURNING ORDER_ID;" ;       
    }
    else if (stod(amount) < 0) { // sell order
      double pos_amount = -stod(amount);
      ss << "SELECT AMOUNT FROM POSITION WHERE ACCOUNT_ID=" << W.quote(user_id) << " AND SYMBOL=" << W.quote(sym) << " AND AMOUNT>=" << pos_amount << " FOR UPDATE;";
      pqxx::result R(W.exec(ss.str()));
      if (R.begin() == R.end()) { // balance insufficient 
        W.abort();
        res_node = res_doc->allocate_node(rapidxml::node_element, "error", "insufficient symbol to sell");
        copy_attr(node, res_doc, res_node);
        return res_node;
      }
      ss.str("");
      ss << "UPDATE POSITION SET AMOUNT=AMOUNT-'" << pos_amount << "' WHERE ACCOUNT_ID=" << W.quote(user_id) << " AND SYMBOL=" << W.quote(sym) << ";";
      ss << "INSERT INTO SYM_ORDER(ACCOUNT_ID, STATUS, SYMBOL, AMOUNT, PRICE) VALUES (" << W.quote(user_id) << ", 'open', " << W.quote(sym) << ", " << W.quote(amount) << ", " << W.quote(limit) << ") RETURNING ORDER_ID;" ;       
    } else { // amount==0 => make no sense
      throw std::exception();
    }
    pqxx::result R2(W.exec(ss.str()));
    W.commit();
    res_node = res_doc->allocate_node(rapidxml::node_element, "opened");
    copy_attr(node, res_doc, res_node);
    int order_id = R2.begin()[0].as<int>();
    char * order_id_char = res_doc->allocate_string(std::to_string(order_id).c_str()); 
    res_node->append_attribute(res_doc->allocate_attribute("id", order_id_char)); 
    try {
      order_match(order_id, C);
      return res_node;
    }
    catch (const pqxx::pqxx_exception & e) {
      //std::cerr << "Match order error: " << e.base().what() << std::endl;
      return res_node;
    }
  }
  catch (const pqxx::pqxx_exception & e) {
    W.abort();
    //std::cerr << "Database Error in <transactions> <order>: " << e.base().what() << std::endl;
    res_node = res_doc->allocate_node(rapidxml::node_element, "error", "Order can not execute by database, might have invalid input.");
    copy_attr(node, res_doc, res_node);
    return res_node;
  }
  catch (const std::exception &e){
    W.abort();
    res_node = res_doc->allocate_node(rapidxml::node_element, "error", "Invalid attr value for order, invalid value or wrong data type");
    copy_attr(node, res_doc, res_node);
    return res_node;
  }
}

void order_match(int order_id, pqxx::connection* C) {
  pqxx::result R; 
  // 1 for seller, 2 for buyer
  int id1, id2, account_id1, account_id2, order_id1, order_id2;
  std::string time1, time2;
  double price, amount1, amount2, price1, price2;
  std::string symbol;
  while (1) {
    std::stringstream ss;
    pqxx::work W(*C);
    ss << "SELECT ID, SYMBOL, AMOUNT, PRICE::numeric, ACCOUNT_ID, EXTRACT(EPOCH FROM CREATE_AT) FROM SYM_ORDER WHERE ORDER_ID=" << order_id << " AND STATUS='open' FOR UPDATE;";
    pqxx::result R = W.exec(ss.str());
    if (R.begin() == R.end()) {
      W.abort();
      return;
    }
    double amount = R.begin()[2].as<double>();
    if (amount < 0) { // seller
      id1 = R.begin()[0].as<int>();
      symbol = R.begin()[1].as<std::string>();
      amount1 = -R.begin()[2].as<double>();
      price1 = R.begin()[3].as<double>();
      account_id1 = R.begin()[4].as<int>();
      time1 = R.begin()[5].as<std::string>();
      order_id1 = order_id;
      ss.str("");
      ss << "SELECT ID, ORDER_ID, AMOUNT, PRICE::numeric, EXTRACT(EPOCH FROM CREATE_AT), ACCOUNT_ID FROM SYM_ORDER WHERE ACCOUNT_ID!=" << account_id1 << "AND STATUS='open' AND SYMBOL="<< W.quote(symbol) << " AND AMOUNT>0 AND PRICE>='" << price1 << "' AND ORDER_ID<'" << order_id1 << "' ORDER BY PRICE DESC, CREATE_AT ASC LIMIT 1 FOR UPDATE;";
      pqxx::result R2 = W.exec(ss.str());
      if (R2.begin() == R2.end()) {
        W.abort();
        return;
      }
      id2 = R2.begin()[0].as<int>();
      order_id2 = R2.begin()[1].as<int>();
      amount2 = R2.begin()[2].as<double>();
      price2 = R2.begin()[3].as<double>();
      time2 = R2.begin()[4].as<std::string>(); 
      account_id2 = R2.begin()[5].as<int>();
    } else { // buyer
      id2 = R.begin()[0].as<int>();
      symbol = R.begin()[1].as<std::string>();
      amount2 = R.begin()[2].as<double>();
      price2 = R.begin()[3].as<double>();
      account_id2 = R.begin()[4].as<int>();
      time2 = R.begin()[5].as<std::string>();
      order_id2 = order_id;
      ss.str("");
      ss << "SELECT ID, ORDER_ID, AMOUNT, PRICE::numeric, EXTRACT(EPOCH FROM CREATE_AT), ACCOUNT_ID FROM SYM_ORDER WHERE ACCOUNT_ID!=" << account_id2 << "AND STATUS='open' AND SYMBOL="<< W.quote(symbol) << " AND AMOUNT<0 AND PRICE<='" << price2 << "' AND ORDER_ID<'" << order_id2 << "' ORDER BY PRICE ASC, CREATE_AT ASC LIMIT 1 FOR UPDATE;";
      pqxx::result R2 = W.exec(ss.str());
      if (R2.begin() == R2.end()) {
        W.abort();
        return;
      }
      id1 = R2.begin()[0].as<int>();
      order_id1 = R2.begin()[1].as<int>();
      amount1 = -R2.begin()[2].as<double>();
      price1 = R2.begin()[3].as<double>();
      time1 = R2.begin()[4].as<std::string>(); 
      account_id1 = R2.begin()[5].as<int>();
    }
    // select amount & price to execute
    amount = amount1 < amount2 ? amount1 : amount2;
    price = stod(time1) < stod(time2) ? price1 : price2;
    // execute both order
    // for buyer: create position(if nessecery); add sym; possibly refund; change order status; possiblely split order
    ss.str("");
    ss << "INSERT INTO POSITION VALUES(" << account_id2 << ", " << W.quote(symbol) << ", 0) ON CONFLICT(ACCOUNT_ID, SYMBOL) DO NOTHING;";
    ss << "UPDATE POSITION SET AMOUNT=AMOUNT+'" << amount << "' WHERE ACCOUNT_ID=" << account_id2 << " AND SYMBOL=" << W.quote(symbol) << ";";
    if (price < price2) {
      if (account_id1 < account_id2) {
        ss << "UPDATE ACCOUNT SET BALANCE=BALANCE+'" << price * amount << "' WHERE ACCOUNT_ID=" << account_id1 << ";";
      }
      ss << "UPDATE ACCOUNT SET BALANCE=BALANCE+'" << (price2 - price) * amount << "' WHERE ACCOUNT_ID=" << account_id2 << ";";
      if (account_id1 >= account_id2) {
        ss << "UPDATE ACCOUNT SET BALANCE=BALANCE+'" << price * amount << "' WHERE ACCOUNT_ID=" << account_id1 << ";";
      }
    } else {
      ss << "UPDATE ACCOUNT SET BALANCE=BALANCE+'" << price * amount << "' WHERE ACCOUNT_ID=" << account_id1 << ";";
    }
    ss << "UPDATE SYM_ORDER SET STATUS='executed', AMOUNT=" << amount << ", PRICE=" << price << ", CREATE_AT=now() WHERE ID=" << id2 << ";";
    if (amount2 > amount) { //need to split order
      ss << "INSERT INTO SYM_ORDER(ORDER_ID, ACCOUNT_ID, STATUS, SYMBOL, AMOUNT, PRICE, CREATE_AT) VALUES (" << order_id2 << ", " << account_id2 << ", 'open', " << W.quote(symbol) << ", " << amount2 - amount << ", " << price2 << ", to_timestamp(" << time2 << "));";
    }
    W.exec(ss.str());
    // for seller: add money; change order status; possiblely split order
    ss.str("");
    // ss << "UPDATE ACCOUNT SET BALANCE=BALANCE+'" << price * amount << "' WHERE ACCOUNT_ID=" << account_id1 << ";";
    ss << "UPDATE SYM_ORDER SET STATUS='executed', AMOUNT=" << -amount << ", PRICE=" << price << ", CREATE_AT=now() WHERE ID=" << id1 << ";";
    if (amount1 > amount) { // need to split order
      ss << "INSERT INTO SYM_ORDER(ORDER_ID, ACCOUNT_ID, STATUS, SYMBOL, AMOUNT, PRICE, CREATE_AT) VALUES (" << order_id1 << ", " << account_id1 << ", 'open', " << W.quote(symbol) << ", " << amount - amount1 << ", " << price1 << ", to_timestamp(" << time1 << "));";
    } 
    W.exec(ss.str());
    W.commit(); 
  }
}

rapidxml::xml_node<>* cancel_order(std::string user_id, rapidxml::xml_node<>* node, rapidxml::xml_document<>* res_doc, pqxx::connection* C) {
  rapidxml::xml_node<>* res_node = nullptr;
  res_node = res_doc->allocate_node(rapidxml::node_element, "canceled");
  rapidxml::xml_attribute<> *attr = node->first_attribute("id");
  if (attr == 0) {
    //std::cerr << "Transaction_Cancel: do not have order id\n";
    return nullptr;
  }
  std::string id(attr->value());
  char * id_ch = res_doc->allocate_string(id.c_str());
  res_node->append_attribute(res_doc->allocate_attribute("id", id_ch)); 
  pqxx::work W(*C);
  pqxx::result R2;
  try {
    std::stringstream ss;
    ss << "UPDATE SYM_ORDER SET STATUS='canceled', CREATE_AT=now() WHERE ORDER_ID=" << W.quote(id) << "AND ACCOUNT_ID=" << W.quote(user_id) << "AND STATUS='open' RETURNING SYMBOL, AMOUNT, PRICE::numeric;";
    pqxx::result R = W.exec(ss.str());
    ss.str("");
    if (R.begin() != R.end()) { // need to refund
      std::string symbol;
      double amount, price;
      symbol = R.begin()[0].as<std::string>();
      amount = R.begin()[1].as<double>();
      price = R.begin()[2].as<double>();
      if (amount < 0) { // refund position
        ss << "UPDATE POSITION SET AMOUNT=AMOUNT+'" << -amount << "' WHERE ACCOUNT_ID=" << W.quote(user_id) << "AND SYMBOL=" << W.quote(symbol) << ";";
      } else { // refund balance
        ss << "UPDATE ACCOUNT SET BALANCE=BALANCE+'" << amount * price << "' WHERE ACCOUNT_ID=" << W.quote(user_id) << ";";
      }
    }
    // print detailed info for order
    ss << "SELECT STATUS, AMOUNT, PRICE::numeric, EXTRACT(EPOCH FROM CREATE_AT) FROM SYM_ORDER WHERE ORDER_ID=" << W.quote(id) << ";";
    R2 = W.exec(ss.str());
    W.commit();
  }
  catch (const pqxx::pqxx_exception & e) {
    W.abort();
    //std::cerr << "Database Error in <transactions> <cancel>: " << e.base().what() << std::endl;
    return nullptr;
  }
  catch (const std::exception &e){
    W.abort();
    //std::cerr << "Error in <transactions> <cancel>: " << e.what() << std::endl;
    return nullptr;
  }
  print_order_status(&R2, res_node, res_doc);
  return res_node;
}

rapidxml::xml_node<>* query_order(std::string user_id, rapidxml::xml_node<>* node, rapidxml::xml_document<>* res_doc, pqxx::connection* C) {
  rapidxml::xml_node<>* res_node = nullptr;
  res_node = res_doc->allocate_node(rapidxml::node_element, "status");
  rapidxml::xml_attribute<> *attr = node->first_attribute("id");
  if (attr == 0) {
    //std::cerr << "Transaction_Query: do not have order id\n";
    return nullptr;
  }
  std::string id(attr->value()); 
  char * id_ch = res_doc->allocate_string(id.c_str());
  res_node->append_attribute(res_doc->allocate_attribute("id", id_ch)); 
  pqxx::work W(*C);
  pqxx::result R2;
  try {
    std::stringstream ss;
    ss << "SELECT STATUS, AMOUNT, PRICE::numeric, EXTRACT(EPOCH FROM CREATE_AT) FROM SYM_ORDER WHERE ORDER_ID=" << W.quote(id) << ";";
    R2 = W.exec(ss.str());
    W.commit();
  }
  catch (const pqxx::pqxx_exception & e) {
    W.abort();
    //std::cerr << "Database Error in <transactions> <query>: " << e.base().what() << std::endl;
    return nullptr;
  }
  catch (const std::exception &e){
    W.abort();
    //std::cerr << "Error in <transactions> <query>: " << e.what() << std::endl;
    return nullptr;
  }
  print_order_status(&R2, res_node, res_doc);
  return res_node;
}

void print_order_status(pqxx::result* R2, rapidxml::xml_node<>* res_node, rapidxml::xml_document<>* res_doc) {
  std::string status, time_str;
  double amt, prc;
  int time;
  for (pqxx::result::const_iterator iter = R2->begin(); iter != R2->end(); ++iter) {
    status = iter[0].as<std::string>();
    amt = iter[1].as<double>();
    prc = iter[2].as<double>();
    time_str = iter[3].as<std::string>();
    time = stoi(time_str);
    if (status == "open") {
      rapidxml::xml_node<>* res_child_node = res_doc->allocate_node(rapidxml::node_element, "open");
      res_node->append_node(res_child_node);
      char * amt_ch = res_doc->allocate_string(std::to_string(abs(amt)).c_str());
      res_child_node->append_attribute(res_doc->allocate_attribute("shares", amt_ch));
    } else if (status == "canceled") {
      rapidxml::xml_node<>* res_child_node = res_doc->allocate_node(rapidxml::node_element, "canceled");
      res_node->append_node(res_child_node);
      char * amt_ch = res_doc->allocate_string(std::to_string(abs(amt)).c_str());
      res_child_node->append_attribute(res_doc->allocate_attribute("shares", amt_ch));
      char * time_ch = res_doc->allocate_string(std::to_string(time).c_str());
      res_child_node->append_attribute(res_doc->allocate_attribute("time", time_ch));
    } else if (status == "executed") {
      rapidxml::xml_node<>* res_child_node = res_doc->allocate_node(rapidxml::node_element, "executed");
      res_node->append_node(res_child_node);
      char * amt_ch = res_doc->allocate_string(std::to_string(abs(amt)).c_str());
      res_child_node->append_attribute(res_doc->allocate_attribute("shares", amt_ch));
      char * prc_ch = res_doc->allocate_string(std::to_string(prc).c_str());
      res_child_node->append_attribute(res_doc->allocate_attribute("price", prc_ch));
      char * time_ch = res_doc->allocate_string(std::to_string(time).c_str());
      res_child_node->append_attribute(res_doc->allocate_attribute("time", time_ch));
    } else {
      //std::cerr << "Invalid status: " << status << std::endl;
    }
  }
}

// Initialize tables.
void create_table(pqxx::connection* C) {
  std::ifstream ifs;
  ifs.open("table.sql", std::ifstream::in);
  std::string line, sql;
  while (getline(ifs, line)) {
    sql += line;
  }
  ifs.close();

  pqxx::work W(*C);
  try {
    W.exec(sql);
    W.commit();
  } catch (const pqxx::pqxx_exception & e) {
    W.abort();
    //std::cerr << "Database Error in <initialization>: " << e.base().what() << std::endl;
  }
}

pqxx::connection* start_connection() {
  pqxx::connection *C;
  try{
    C = new pqxx::connection("host=db port=5432 dbname=STOCK_MARKET user=postgres password=passw0rd");
    if (C->is_open()) {
      //std::cout << "Opened database successfully: " << C->dbname() << std::endl;
    } else {
      std::cerr << "Can't open database" << std::endl;
      return nullptr;
    }
  } catch (const pqxx::pqxx_exception & e) {
    std::cerr << e.base().what() << std::endl;
    return nullptr;
  } catch (const std::exception &e){
    std::cerr << e.what() << std::endl;
    return nullptr;
  }

  return C;
}

void end_connection(pqxx::connection* C) {
  C->disconnect();
}
