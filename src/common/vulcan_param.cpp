// Copyright 2023 VulcanDB

#include "common/vulcan_param.h"

#include <unistd.h>

#include <filesystem>
#include <string>

namespace vulcan {

std::string process_name(const char* prog_name) {
  std::string process_name_ = prog_name;
  std::size_t pos = process_name_.find_last_of('/');
  if (pos != std::string::npos) {
    process_name_ = process_name_.substr(pos + 1);
  }
  return process_name_ + "-" + std::to_string(getpid());
}

VulcanParam* VulcanParam::get_instance() {
  static VulcanParam instance;
  return &instance;
}

void VulcanParam::default_init(const char* prog_name) {
  process_name_ = process_name(prog_name);
  home_ = std::filesystem::path(getenv("HOME")) / ".vulcandb";
  data_dir_ = home_ / "data";
  log_dir_ = home_ / "log";
  conf_file_ = "/etc/vulcandb.conf";
}

}  // namespace vulcan
