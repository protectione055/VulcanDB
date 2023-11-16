// Copyright 2023 VulcanDB
#pragma once

#include <filesystem>
#include <string>

namespace vulcan {

class VulcanConfig {
 public:
  ~VulcanConfig() = default;

  static VulcanConfig* get_instance() {
    static VulcanConfig instance;
    return &instance;
  }

  static void set_home(const std::filesystem::path& home) {
    default_home_ = home;
  }
  static void set_data_dir(const std::filesystem::path& data_dir) {
    default_data_dir_ = data_dir;
  }
  static void set_log_dir(const std::filesystem::path& log_dir) {
    default_log_dir_ = log_dir;
  }
  static std::filesystem::path get_home() { return default_home_; }
  static std::filesystem::path get_data_dir() { return default_data_dir_; }
  static std::filesystem::path get_log_dir() { return default_log_dir_; }

 private:
  static std::filesystem::path default_home_;
  static std::filesystem::path default_data_dir_;
  static std::filesystem::path default_log_dir_;

  VulcanConfig() = default;
  VulcanConfig(const VulcanConfig&) = delete;
};

}  // namespace vulcan
