// Copyright 2023-2024 VulcanDB

#pragma once

#include <fstream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "express_defs.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

extern "C" int express_parse(const char* s, void* schema);

std::string to_lower(const std::string& str);

class Logger {
 public:
  ~Logger() = default;

  static Logger* get_instance() {
    static Logger logger;
    return &logger;
  }

  template <typename... Args>
  void trace(const char* fmt, const Args&... args) {
    logger_->trace(fmt, args...);
  }

  template <typename... Args>
  void debug(const char* fmt, const Args&... args) {
    logger_->debug(fmt, args...);
  }

  template <typename... Args>
  void info(const char* fmt, const Args&... args) {
    logger_->info(fmt, args...);
  }

  template <typename... Args>
  void error(const char* fmt, const Args&... args) {
    logger_->error(fmt, args...);
  }

  enum class Level { trace, debug, info, error };

  void set_level(Level level) {
    switch (level) {
      case Level::trace:
        logger_->set_level(spdlog::level::trace);
        break;
      case Level::debug:
        logger_->set_level(spdlog::level::debug);
        break;
      case Level::info:
        logger_->set_level(spdlog::level::info);
        break;
      case Level::error:
        logger_->set_level(spdlog::level::err);
        break;

      default:
        break;
    }
  }

 private:
  std::unique_ptr<spdlog::logger> logger_;

 private:
  Logger() {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    spdlog::sinks_init_list sink_list = {console_sink};

    logger_ = std::make_unique<spdlog::logger>("logger", sink_list);
    logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");
    logger_->set_level(spdlog::level::trace);
  }
};

extern Logger* logger;

struct ExpressParserContext {
 public:
  ExpressParserContext() { schema = std::shared_ptr<Schema>(new_schema()); }
  std::map<std::string, int> type_id_index;
  std::map<std::string, int> entity_id_index;
  std::shared_ptr<Schema> schema;

  void clear() {
    type_id_index.clear();
    entity_id_index.clear();
    schema = std::shared_ptr<Schema>(new_schema());
  }
};

class ExpressParser {
 public:
  ExpressParser() = default;
  ~ExpressParser() = default;

  std::shared_ptr<ExpressParserContext> parse(const char* exp_file);
  
  void init_type(TypeArgs* args);
  void init_entity(EntityArgs* args);

 private:
  typedef struct Token {
   public:
    Token(const char* content, size_t start)
        : content(content), start(start), end(start) {}

    bool operator==(const char* other) {
      if (strlen(other) != get_length()) {
        return false;
      }

      size_t i = 0;
      while (other[i] != '\0' && content[start + i] != '\0') {
        if (content[start + i] != other[i]) {
          return false;
        }
        i++;
      }
      return true;
    }

    bool is_valid() { return content[start] != '\0'; }
    void set_start(size_t start) { this->start = start; }
    void set_end(size_t end) { this->end = end; }
    size_t get_start() { return start; }
    size_t get_end() { return end; }
    size_t get_length() { return end - start; }

    bool has_next() {
      for (int i = end; content[i] != '\0'; i++) {
        if (content[i] != ' ' && content[i] != '\n' && content[i] != '\t' &&
            content[i] != '\0' && content[i] != ';') {
          return true;
        }
      }
      return false;
    }

    Token next() {
      size_t i = end;
      while (content[i] == ' ' || content[i] == '\n' || content[i] == '\t' ||
             content[i] == ';') {
        i++;
      }
      start = i;
      while (content[i] != ' ' && content[i] != '\n' && content[i] != '\0' &&
             content[i] != '\t' && content[i] != ';') {
        i++;
      }
      end = i;
      return *this;
    }

    std::string to_string() {
      return std::string(content + start, content + end);
    }

   private:
    const char* content;
    size_t start;
    size_t end;
  } Token;

  /*
   * 对EXP文件进行预处理，获取类型和实体的声明
   *
   */
  void preprocess(const char* content, std::shared_ptr<Schema> schema);

  std::string create_pgsql_for_enum(Type* type);

 private:
  std::shared_ptr<ExpressParserContext> context_;
};

class TopoGraph {
 public:
  TopoGraph() = default;
  ~TopoGraph() = default;

  void add_node(int node_id);
  void add_edge(int from, int to);
  std::vector<int> topo_sort();

 private:
  std::map<int, std::set<int>> graph_;
  std::map<int, int> in_degree_;
};
