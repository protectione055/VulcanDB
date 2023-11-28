// Copyright 2023 VulcanDB
#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "common/defs.h"
#include "common/vulcan_logger.h"


namespace vulcan {

class VulcanParam {
 public:
  ~VulcanParam() = default;

  static VulcanParam* get_instance();

  void init(const char* prog_name);

  /**
   * @brief Loads the configuration file and updates the parameter settings.
   *
   * This function loads the specified configuration file and updates the
   * parameter settings based on the values specified in the file. If the file
   * fails to load, a warning message is logged. The function uses the default
   * configuration map to retrieve the values from the file, and updates the
   * corresponding entries in the configuration map.
   *
   * @param conf_file The path to the configuration file.
   */
  void load_conf_file();
  void set_conf_file(const std::string& conf_file) { conf_file_ = conf_file; }

  /**
   * Retrieves the value associated with the specified key from the
   * configuration map.
   *
   * @param key The key to retrieve the value for.
   * @return The value associated with the key, or an empty string if the
   * key is not found.
   */
  std::string get(const std::string& key) {
    auto it = conf_map_.find(key);
    if (it != conf_map_.end()) {
      return it->second;
    }
    return "";
  }

  /**
   * @brief Sets the value of a configuration parameter.
   *
   * @param key The key of the configuration parameter.
   * @param value The value to be set.
   */
  void set(const std::string& key, const std::string& value) {
    conf_map_[key] = value;
  }

  /**
   * @brief 获取最大连接数
   *
   * @return int 最大连接数
   */
  int get_max_connection_num() {
    return std::stoi(conf_map_[MAX_CONNECTION_NUM]);
  }

  LOG_LEVEL get_log_level() {
    return log_levels_.at(std::stoi(conf_map_[VULCAN_LOG_LEVEL]));
  }

  LOG_LEVEL get_console_log_level() {
    return log_levels_.at(std::stoi(conf_map_[VULCAN_CONSOLE_LOG_LEVEL]));
  }

  std::string get_process_name() const { return process_name_; }
  int get_server_port() { return std::stoi(conf_map_[VULCAN_PORT]); }

 private:
  VulcanParam();
  VulcanParam(const VulcanParam&) = delete;
  void check_and_create_dir(const char* var_name,
                            const std::filesystem::path& path);

 private:
  std::string process_name_;
  std::string conf_file_ = DEFAULT_CONF_FILE;
  std::map<std::string, std::string> conf_map_;  // 所有配置项，以kv形式记录

 private:
  // 默认配置项
  const std::map<std::string, std::string> default_conf_map_ = {
      {VULCAN_HOME, DEFAULT_HOME},
      {VULCAN_DATA_DIR, DEFAULT_DATA_DIR},
      {VULCAN_LOG_DIR, DEFAULT_LOG_DIR},
      {VULCAN_LOG_LEVEL, DEFAULT_LOG_LEVEL},
      {VULCAN_CONSOLE_LOG_LEVEL, DEFAULT_LOG_LEVEL},
      {VULCAN_PORT, PORT_DEFAULT},
      {VULCAN_UNIX_SOCKET_PATH, UNIX_SOCKET_PATH_DEFAULT},
      {MAX_CONNECTION_NUM, MAX_CONNECTION_NUM_DEFAULT}};

  // 日志级别
  const std::vector<LOG_LEVEL> log_levels_ = {
      LOG_LEVEL::PANIC, LOG_LEVEL::ERR,   LOG_LEVEL::WARN, LOG_LEVEL::INFO,
      LOG_LEVEL::INFO,  LOG_LEVEL::DEBUG, LOG_LEVEL::TRACE};
};

}  // namespace vulcan
