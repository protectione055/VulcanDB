// Copyright 2023 VulcanDB

#pragma once

#include <pthread.h>
#include <unistd.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>

#include "common/defs.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace vulcan {

typedef enum {
  PANIC = 0,
  ERR = 1,
  WARN = 2,
  INFO = 3,
  DEBUG = 4,
  TRACE = 5,
  LAST
} LOG_LEVEL;

class VulcanLogger {
 public:
  ~VulcanLogger() = default;

  static VulcanLogger* get_instance() {
    static VulcanLogger instance;
    return &instance;
  }

  void init(const std::filesystem::path& log_dir, const std::string& log_name,
            LOG_LEVEL log_level = INFO, LOG_LEVEL console_level = WARN);

  bool is_init() const { return is_init_; }
  LOG_LEVEL get_log_level() const { return log_level_; }

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
  void warn(const char* fmt, const Args&... args) {
    logger_->warn(fmt, args...);
  }

  template <typename... Args>
  void error(const char* fmt, const Args&... args) {
    logger_->error(fmt, args...);
  }

  template <typename... Args>
  void panic(const char* fmt, const Args&... args) {
    logger_->critical(fmt, args...);
  }

 private:
  VulcanLogger() = default;

  bool is_init_ = false;
  std::unique_ptr<spdlog::logger> logger_;
  std::filesystem::path log_dir_;
  std::filesystem::path log_file_;
  LOG_LEVEL log_level_;
  LOG_LEVEL console_level_;
  std::map<LOG_LEVEL, spdlog::level::level_enum> log_level_map_ = {
      {PANIC, spdlog::level::critical}, {ERR, spdlog::level::err},
      {WARN, spdlog::level::warn},      {INFO, spdlog::level::info},
      {DEBUG, spdlog::level::debug},    {TRACE, spdlog::level::trace}};
};

/*
 * VULCAN_LOG(level, fmt, ...)
 * @param level: log level. e.g. trace, info, warn, error, debug, panic
 * @param fmt: format string. e.g. "hello {}"
 * @param ...: format string arguments. e.g. "world"
 */
#define VULCAN_LOG(level, fmt, ...)                                    \
  do {                                                                 \
    if (vulcan::VulcanLogger::get_instance()->is_init()) {             \
      vulcan::VulcanLogger::get_instance()->level(fmt, ##__VA_ARGS__); \
    } else {                                                           \
      spdlog::info(fmt, ##__VA_ARGS__);                               \
    }                                                                  \
  } while (0);

}  // namespace vulcan
