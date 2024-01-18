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

std::string FileManager::read_file(const std::string& filename) {
  std::ifstream ifs(filename);
  if (!ifs.is_open()) {
    logger->error("Failed to open {}", filename);
    return "";
  }
  std::stringstream buffer;
  buffer << ifs.rdbuf();
  std::string content = buffer.str();

  return content;
}

void FileManager::write_file(const std::string& filename,
                             const std::string& content) {
  std::ofstream ofs(filename);
  if (!ofs.is_open()) {
    logger->error("Failed to open {}", filename);
    return;
  }
  ofs << content;
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

std::string PgSQLGenerator::create_pgsql_for_enum(Type* type) {
  std::string sql =
      "CREATE TYPE " + std::string(type->desc.name) + " AS ENUM (";
  for (int i = 0; i < type->enum_num; i++) {
    sql += "'" + std::string(type->enum_list[i]) + "'";
    if (i != type->enum_num - 1) {
      sql += ", ";
    }
  }
  sql += ");";

  return sql;
}

std::string PgSQLGenerator::typecast_express_to_sql(Type* type) {
  std::string res;
  switch (type->desc.type) {
    case T_INTEGER:
      res = "INTEGER";
      break;
    case T_NUMBER:
      res = "NUMERIC";
      break;
    case T_REAL:
      res = "REAL";
      break;
    case T_BINARY:
    case T_LOGICAL:
    case T_STRING:
      res = "VARCHAR";
      break;
    case T_BOOLEAN:
      res = "BOOLEAN";
      break;
    case T_ENUM:
      res = std::string(type->desc.name);
      break;
    case T_SELECT:
      res = "SPF_SELECT";
      break;
    case T_REFERENCE:
      res = "OID";
      break;
    case T_DERIVED:
      res = typecast_express_to_sql((Type*)type->ref_type);
      break;
    default:
      logger->error("Invalid type: {}", type->desc.type);
      exit(1);
  }
  if (type->is_list) {
    res += "[]";
  }
  return res;
}

std::string PgSQLGenerator::create_pgsql_for_entity(Entity* entity) {
  std::string sql = "CREATE FUNCTION create_" + to_lower(entity->desc.name) +
                    "() RETURNS BOOLEAN" + +" AS $$\n";
  sql += "BEGIN\n";

  if (entity->supertype != NULL) {
    sql += "PERFORM create_" + to_lower(entity->supertype->desc.name) + "();\n";
  }

  sql += "CREATE TABLE IF NOT EXISTS " + to_lower(entity->desc.name) + "(";

  // 处理属性类型映射
  for (int i = 0; i < entity->attr_num; i++) {
    sql += " \"" + std::string(entity->attr_name[i]) + "\" ";
    sql += typecast_express_to_sql(entity->attr_type[i]);
    if (!entity->is_optional[i]) {
      sql += " NOT NULL";
    }
    if (i != entity->attr_num - 1) {
      sql += ", ";
    }
  }
  sql += ")";

  // 处理继承关系
  if (entity->supertype != NULL) {
    sql += " INHERITS (" + std::string(entity->supertype->desc.name) + ")";
  }
  sql += ";\n";

  sql += "RETURN TRUE;\n";
  sql += "END;\n";
  sql += "$$ LANGUAGE plpgsql;";

  return sql;
}

std::string PgSQLGenerator::generate_pgsql_for_schema(
    std::shared_ptr<Schema> schema) {
  std::string res;

  // 增加spf_type的枚举类型
  res += "CREATE TYPE express_type AS ENUM (";
  res += "'INTEGER', 'REAL', 'STRING', 'BOOLEAN', 'ENUM', 'SELECT', 'ENTITY'";
  res += ");\n";

  // 增加SELECT类型
  res += "CREATE TYPE spf_select AS (";
  res += "type express_type,";
  res += "value bytea,";
  res += "label text";
  res += ");\n";

  // 生成EXPRESS枚举类型的SQL
  for (int i = 0; i < schema->type_num; i++) {
    if (schema->types[i]->desc.type == T_ENUM) {
      res += create_pgsql_for_enum(schema->types[i]);
      res += "\n";
    }
  }

  // 对表进行拓扑排序，然后生成带继承关系的表
  TopoGraph topo_graph;
  for (int i = 0; i < schema->entity_num; i++) {
    for (int j = 0; j < schema->entities[i]->subtype_num; j++) {
      Entity* subtype = schema->entities[i]->subtypes[j];
      topo_graph.add_edge(i, get_entity_id(subtype));
    }
  }
  std::vector<int> topo_order = topo_graph.topo_sort();
  Entity* entity = find_entity_by_id(schema.get(), topo_order[0]);
  assert(entity->supertype == NULL);
  for (auto& entity_id : topo_order) {
    Entity* entity = find_entity_by_id(schema.get(), entity_id);
    res += create_pgsql_for_entity(entity);
    res += "\n";
  }

  return res;
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

int main() {
  std::string file;

  logger->set_level(Logger::Level::error);

  std::cout << "input file: ";
  std::cin >> file;
  std::string content = FileManager::read_file(file);
  getchar();

  ExpressParser parser;
  std::shared_ptr<ExpressParserContext> parse_context =
      parser.parse(content.c_str());
  assert(parse_context->entity_id_index.size() ==
         parse_context->schema->entity_num);
  PgSQLGenerator pgsql_generator(parse_context);

  std::string command = "";
  while (true) {
    std::cout << "input command: ";
    std::getline(std::cin, command);
    if (command == "exit") {
      break;
    } else if (command.substr(0, 4) == "type") {
      std::string type_name = command.substr(command.find_first_of(" ") + 1);
      size_t type_id = parse_context->type_id_index[type_name];
      Type* type = parse_context->schema->types[type_id];
      char* type_tree = serialize_type_tree(type);
      printf("%s\n", type_tree);
      free(type_tree);
    } else if (command.substr(0, strlen("entity")) == "entity") {
      std::string entity_name = command.substr(command.find_first_of(" ") + 1);
      size_t entity_id = parse_context->entity_id_index[entity_name];
      Entity* entity = parse_context->schema->entities[entity_id];
      printf("\033[32m%s\033[0m\n", entity->desc.name);
      // 打印entity的父类
      if (entity->supertype != NULL) {
        printf("Supertypes: \n");
        printf("\033[33m%s\033[0m\n", entity->supertype->desc.name);
      }
      // 打印entity的子类
      printf("Subtypes: \n");
      for (int i = 0; i < entity->subtype_num; i++) {
        printf("\033[31m%s\033[0m, ", entity->subtypes[i]->desc.name);
      }
      // 打印entity的属性
      printf("\nAttributes: \n");
      std::stack<Entity*> entity_stack;
      Entity* cur_entity = entity;
      while (cur_entity != NULL && entity->supertype != NULL) {
        entity_stack.push(cur_entity);
        cur_entity = cur_entity->supertype;
      }
      while (!entity_stack.empty()) {
        cur_entity = entity_stack.top();
        entity_stack.pop();
        printf("Inherits from %s\n", cur_entity->desc.name);
        for (int i = 0; i < cur_entity->attr_num; i++) {
          printf("\033[34m%s\033[0m : %s\n", cur_entity->attr_name[i],
                 serialize_type_tree(cur_entity->attr_type[i]));
        }
      }
    } else if (command == "sql") {
      std::string dest_path = "../../resources/express/";
      size_t basename_len = file.find_last_of(".") - file.find_last_of("/");
      dest_path += file.substr(file.find_last_of("/"), basename_len) + ".sql";
      const std::string res =
          pgsql_generator.generate_pgsql_for_schema(parse_context->schema);
      FileManager::write_file(dest_path, res);
      std::cout << "SQL file generated." << dest_path << std::endl;
    } else {
      std::cout << "Invalid command: " << command << std::endl;
    }
  }

  return 0;
}
