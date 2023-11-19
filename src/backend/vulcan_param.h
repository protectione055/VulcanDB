// Copyright 2023 VulcanDB
#pragma once

#include <filesystem>
#include <string>

#include "common/vulcan_logger.h"

namespace vulcan {

class VulcanParam {
 public:
  ~VulcanParam() = default;

  static VulcanParam* get_instance();

  void default_init(const char* prog_name);

  void set_home(const std::filesystem::path& home) { home_ = home; }
  void set_data_dir(const std::filesystem::path& data_dir) {
    data_dir_ = data_dir;
  }
  void set_log_dir(const std::filesystem::path& log_dir) { log_dir_ = log_dir; }
  void set_conf_file(const std::filesystem::path& conf_file) {
    conf_file_ = conf_file;
  }
  void set_server_port(int port) { server_port_ = port; }
  void set_unix_socket_path(const std::filesystem::path& unix_socket_path) {
    unix_socket_path_ = unix_socket_path;
  }
  void set_log_level(LOG_LEVEL log_level) { log_level_ = log_level; }
  void set_console_level(LOG_LEVEL console_level) {
    console_log_level_ = console_level;
  }

  std::string get_process_name() const { return process_name_; }
  std::filesystem::path get_home() const { return home_; }
  std::filesystem::path get_data_dir() const { return data_dir_; }
  std::filesystem::path get_log_dir() const { return log_dir_; }
  std::filesystem::path get_conf_file() const { return conf_file_; }
  std::filesystem::path get_unix_socket_path() const {
    return unix_socket_path_;
  }
  LOG_LEVEL get_log_level() const { return log_level_; }
  LOG_LEVEL get_console_log_level() const { return console_log_level_; }

 private:
  std::string process_name_;
  std::filesystem::path home_;              // ~/.vulcandb
  std::filesystem::path data_dir_;          // 数据文件目录
  std::filesystem::path log_dir_;           // 日志文件目录
  std::filesystem::path conf_file_;         // 配置文件路径
  std::filesystem::path unix_socket_path_;  // unix socket路径
  LOG_LEVEL log_level_;                     // 日志级别
  LOG_LEVEL console_log_level_;             // 控制台日志级别
  int server_port_;                         // 服务器端口
  //   bool is_demon_ = false;                   // 是否以守护进程方式运行

  VulcanParam() = default;
  VulcanParam(const VulcanParam&) = delete;
};

}  // namespace vulcan
