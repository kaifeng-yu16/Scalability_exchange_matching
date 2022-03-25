#ifndef REQUEST__H
#define REQUEST__H

#include <exception>
#include <cstring>
#include <iostream>
#include <pqxx/pqxx>
#include <string>
#include <sstream>
#include "../lib/rapidxml_ext.hpp"

std::string execute_request(std::string req, pqxx::connection* C);

std::string create_req(rapidxml::xml_node<>* root, pqxx::connection* C);

std::string transaction_req(rapidxml::xml_node<>* root, pqxx::connection* C);

rapidxml::xml_node<>* add_new_order(std::string user_id, rapidxml::xml_node<>* node, rapidxml::xml_document<>* res_doc, pqxx::connection* C);

rapidxml::xml_node<>* cancel_order(std::string user_id, rapidxml::xml_node<>* node, rapidxml::xml_document<>* res_doc, pqxx::connection* C);

rapidxml::xml_node<>* query_order(std::string user_id, rapidxml::xml_node<>* node, rapidxml::xml_document<>* res_doc, pqxx::connection* C);

void copy_attr(rapidxml::xml_node<>* node, rapidxml::xml_document<>* res_doc, rapidxml::xml_node<>* res_node);

void order_match(int order_id, pqxx::connection* C);
#endif


