// Copyright 2023 VulcanDB

#pragma once

#include <unistd.h>

#include <memory>
#include <string>

#include "common/globals.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"

#ifdef NDEBUG
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#else
#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#endif

namespace vulcan {

class VulcanLogger {
 public:
  ~VulcanLogger() = default;

  static VulcanLogger& get_instance() {
    static VulcanLogger instance;
    return instance;
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

 private:
  VulcanLogger() {
    try {
      if (!std::filesystem::exists(VulcanConfig::get_log_dir())) {
        std::filesystem::create_directories(VulcanConfig::get_log_dir());
      }

      std::filesystem::path log_file_path =
          VulcanConfig::get_log_dir() /
          ("vulcan_log_" + std::to_string(getpid()) + ".log");
      logger_ = spdlog::basic_logger_mt("vulcan_logger", log_file_path.c_str());
    } catch (const spdlog::spdlog_ex& ex) {
      std::cout << "spdlog init failed: " << ex.what() << std::endl;
    } catch (const std::exception& ex) {
      std::cout << "VulcanLogger init failed: " << ex.what() << std::endl;
    }
  }

  VulcanLogger(const VulcanLogger&) = delete;

  std::shared_ptr<spdlog::logger> logger_ = nullptr;
};

}  // namespace vulcan
