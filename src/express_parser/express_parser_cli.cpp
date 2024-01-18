// Copyright 2024 VulcanDB

#include "express_parser_cli.h"

#include <unistd.h>

#include <iostream>
#include <stack>
#include <memory>

#include "express_defs.h"
#include "express_parser.h"

extern Logger* logger;

std::string FileManager::read_file(const std::string& filename) {
  std::ifstream ifs(filename);
  if (!ifs.is_open()) {
    logger->error("Failed to open {}", filename);
    exit(1);
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

int main(int argc, char* argv[]) {
  logger->set_level(Logger::Level::error);

  std::string file;
  int opt;
  while ((opt = getopt(argc, argv, "f:h")) != -1) {
    switch (opt) {
      case 'f':
        file = optarg;
        break;
      case 'h':
        std::cout << "Usage: " << argv[0] << " -f <file.exp>" << std::endl;
        break;
      default:
        std::cout << "Invalid option: " << opt << std::endl;
        break;
    }
  }

  std::string content = FileManager::read_file(file);

  ExpressParser parser;
  std::shared_ptr<ExpressParserContext> parse_context =
      parser.parse(content.c_str());
  assert(parse_context->entity_id_index.size() ==
         parse_context->schema->entity_num);
  PgSQLGenerator pgsql_generator(parse_context);

  std::string command = "";
  while (true) {
    std::cout << "--> ";
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
      printf("%s", cur_entity->desc.name);
      for (int i = 0; i < entity->attr_num; i++) {
        printf("\033[34m%s\033[0m : %s\n", entity->attr_name[i],
               serialize_type_tree(entity->attr_type[i]));
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
