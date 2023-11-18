// Copyright 2023 VulcanDB

#pragma once

#include <unistd.h>

#include <filesystem>
#include <memory>
#include <string>

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/spdlog.h"

// #ifdef NDEBUG
// #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
// #else
// #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
// #endif

namespace vulcan {
class VulcanLogger {
 public:
  ~VulcanLogger() = default;

  static VulcanLogger* get_instance() {
    static VulcanLogger instance;
    return &instance;
  }

  void init(const std::filesystem::path& log_dir, const std::string& log_name) {
    try {
      if (!std::filesystem::exists(log_dir)) {
        std::filesystem::create_directories(log_dir);
      }

      std::filesystem::path log_file_path = log_dir / (log_name + ".log");
      logger_ = std::move(
          spdlog::basic_logger_mt("vulcan_logger", log_file_path.string()));
    } catch (const spdlog::spdlog_ex& ex) {
      std::cout << "spdlog init failed: " << ex.what() << std::endl;
    } catch (const std::exception& ex) {
      std::cout << "VulcanLogger init failed: " << ex.what() << std::endl;
    }
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
  VulcanLogger() = default;
  VulcanLogger(const VulcanLogger&) = delete;

  std::shared_ptr<spdlog::logger> logger_;
  std::filesystem::path log_dir_;
};

}  // namespace vulcan
