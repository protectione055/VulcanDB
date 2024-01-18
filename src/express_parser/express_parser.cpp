// Copyright 2023-2024 VulcanDB

#include "express_parser.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <sstream>
#include <stack>
#include <string>

#include "express_defs.h"

extern "C" int express_parse(const char* s, void* schema);

std::string to_lower(const std::string& str) {
  std::string res = str;
  std::transform(res.begin(), res.end(), res.begin(),
                 [](unsigned char c) { return std::tolower(c); });
  return res;
}

Logger* logger = Logger::get_instance();

/*
 * 语法分析器中通过解析得到的参数构造一个Type，将解析出的Type的参数与schema中的类型声明绑定
 * @param parser ExpressParser对象的指针
 * @param type
 * 从express定义解析得到的类型参数结构体，用于Type内部表示的参数初始化
 *
 */
extern "C" void init_type(void* parser, TypeArgs* args) {
  ExpressParser* express_parser = reinterpret_cast<ExpressParser*>(parser);
  express_parser->init_type(args);
}

extern "C" void init_entity(void* parser, EntityArgs* args) {
  ExpressParser* express_parser = reinterpret_cast<ExpressParser*>(parser);
  express_parser->init_entity(args);
}

std::shared_ptr<ExpressParserContext> ExpressParser::parse(
    const char* exp_file) {
  context_ = std::make_shared<ExpressParserContext>();

  preprocess(exp_file, context_->schema);

  int result = express_parse(exp_file, reinterpret_cast<void*>(this));
  if (result != 0) {
    logger->error("express_parse with code {}.", result);
  }

  logger->info("parse successed.");

  return context_;
}

void ExpressParser::init_type(TypeArgs* args) {
  auto& type_id_index = context_->type_id_index;
  auto& entity_id_index = context_->entity_id_index;
  std::string type_name = args->type_name;

  Type* type_decl = context_->schema->types[type_id_index[type_name]];
  assign_type_args(type_decl, args);
}

void ExpressParser::init_entity(EntityArgs* args) {
  int entity_idx = context_->entity_id_index[args->entity_name];
  Entity* entity_decl = context_->schema->entities[entity_idx];

  assert(strcmp(entity_decl->desc.name, args->entity_name) == 0);
  assign_entity_args(entity_decl, args);

  logger->debug("init entity: {}", args->entity_name);
}

void ExpressParser::preprocess(const char* exp_file,
                               std::shared_ptr<Schema> schema) {
  const char* TYPE_STR = "TYPE";
  const char* ENTITY_STR = "ENTITY";

  Token token(exp_file, 0);
  while (token.has_next()) {
    token = token.next();
    assert(token.is_valid());

    if (token == TYPE_STR) {
      token = token.next();
      if (!token.is_valid()) {
        logger->error("Invalid syntax: null after TYPE");
        exit(1);
      }

      std::string type_name = token.to_string();
      logger->trace("declaration of type={}", type_name);
      add_type_to_schema(schema.get(), type_name.c_str());
      context_->type_id_index[type_name] = schema->type_num - 1;
    } else if (token == ENTITY_STR) {
      token = token.next();
      if (!token.is_valid()) {
        logger->error("Invalid syntax: null after ENTITY");
        exit(1);
      }
      std::string entity_name = token.to_string();
      logger->trace("declaration of entity={}", entity_name);
      add_entity_to_schema(schema.get(), entity_name.c_str());
      context_->entity_id_index[entity_name] = schema->entity_num - 1;
    }
  }
  logger->info("preprocess successed.");
}

void TopoGraph::add_node(int node_id) {
  if (graph_.find(node_id) == graph_.end()) {
    graph_[node_id] = std::set<int>();
    in_degree_[node_id] = 0;
  }
}

void TopoGraph::add_edge(int from, int to) {
  add_node(from);
  add_node(to);
  const auto ret = graph_[from].insert(to);
  if (ret.second) {
    in_degree_[to]++;
  }
}

std::vector<int> TopoGraph::topo_sort() {
  std::vector<int> res;

  std::queue<int> q;
  for (auto item : in_degree_) {
    int node = item.first;
    int degree = item.second;

    if (degree == 0) {
      q.push(node);
    }
  }
  while (!q.empty()) {
    int node = q.front();
    q.pop();
    res.push_back(node);
    for (auto& edge : graph_[node]) {
      in_degree_[edge]--;
      if (in_degree_[edge] == 0) {
        q.push(edge);
      }
    }
  }
  return res;
}
