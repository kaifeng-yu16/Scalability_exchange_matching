#ifndef REQUEST__H
#define REQUEST__H

#include <string>
#include <cstring>
#include <cstdlib>
#include <exception>
#include <sstream>
#include <fstream>
#include <iostream>
#include <pqxx/pqxx>
#include "../lib/rapidxml_ext.hpp"

std::string execute_request(std::string req, pqxx::connection* C);
pqxx::connection* start_connection();
void end_connection(pqxx::connection* C);

std::string create_req(rapidxml::xml_node<>* root, pqxx::connection* C);
std::string transaction_req(rapidxml::xml_node<>* root, pqxx::connection* C);

rapidxml::xml_node<>* add_new_order(std::string user_id, rapidxml::xml_node<>* node, rapidxml::xml_document<>* res_doc, pqxx::connection* C);
rapidxml::xml_node<>* cancel_order(std::string user_id, rapidxml::xml_node<>* node, rapidxml::xml_document<>* res_doc, pqxx::connection* C);
rapidxml::xml_node<>* query_order(std::string user_id, rapidxml::xml_node<>* node, rapidxml::xml_document<>* res_doc, pqxx::connection* C);

void order_match(int order_id, pqxx::connection* C);
void copy_attr(rapidxml::xml_node<>* node, rapidxml::xml_document<>* res_doc, rapidxml::xml_node<>* res_node);
void print_order_status(pqxx::result* R2, rapidxml::xml_node<>* res_node, rapidxml::xml_document<>* res_doc); 
void create_table(pqxx::connection* C);
void initialize_table(pqxx::connection* C);

#endif
