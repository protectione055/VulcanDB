// Copyright 2023 VulcanDB
#include "common/vulcan_logger.h"

#include <stdarg.h>

#include <iostream>
#include <utility>

#include "common/defs.h"
#include "spdlog/sinks/rotating_file_sink.h"  // support for rotating file logging
#include "spdlog/sinks/stdout_color_sinks.h"  // or "../stdout_sinks.h" if no colors needed
#include "spdlog/spdlog.h"

namespace vulcan {

// 初始化日志系统
// log_dir: 日志文件所在目录
// log_name: 日志文件名
void VulcanLogger::init(const std::filesystem::path& log_dir,
                        const std::string& log_name, LOG_LEVEL log_level,
                        LOG_LEVEL console_level) {
  try {
    if (!std::filesystem::exists(log_dir)) {
      std::filesystem::create_directories(log_dir);
    }

    log_level_ = log_level;
    console_level_ = console_level;
    log_dir_ = log_dir;
    log_file_ = log_dir / (log_name + ".log");

    auto file_sink =
        std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_file_, true);
    file_sink->set_level(log_level_map_[log_level_]);

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(log_level_map_[console_level_]);

    spdlog::sinks_init_list sink_list = {file_sink, console_sink};

    logger_ = std::make_unique<spdlog::logger>("vulcan_logger", sink_list);
    logger_->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");
    logger_->flush_on(spdlog::level::trace);
    is_init_ = true;
  } catch (const spdlog::spdlog_ex& ex) {
    std::cerr << "spdlog init failed: " << ex.what() << std::endl;
  } catch (const std::exception& ex) {
    std::cerr << "VulcanLogger init failed: " << ex.what() << std::endl;
  }
}

}  // namespace vulcan
