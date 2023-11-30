// Copyright 2023 VulcanDB

#include "backend/vulcan_param.h"

#include <unistd.h>

#include <filesystem>
#include <string>

#include "common/ini_parser.h"
#include "common/os.h"
#include "common/vulcan_logger.h"

namespace vulcan {

std::string process_name(const char* prog_name) {
  std::string process_name_ = prog_name;
  std::size_t pos = process_name_.find_last_of('/');
  if (pos != std::string::npos) {
    process_name_ = process_name_.substr(pos + 1);
  }
  return process_name_;
}

VulcanParam::VulcanParam() {}

void VulcanParam::load_conf_file() {
  IniFile ini_file;
  if (ini_file.load(conf_file_) != 0) {
    LOG(warn, "Failed to load config file: {}", conf_file_);
  } else {
    // load settings from config file, if any
    for (auto& entry : default_conf_map_) {
      conf_map_[entry.first] =
          ini_file.get(entry.first, entry.second, BASE_SECTION_NAME);
    }
  }
}

VulcanParam* VulcanParam::get_instance() {
  static VulcanParam instance;
  return &instance;
}

void VulcanParam::check_and_create_dir(const char* var_name,
                                       const std::filesystem::path& path) {
  try {
    std::printf("%s=%s\n", var_name, path.string().c_str());

    std::filesystem::path definite_path = get_definite_path(path);
    set(var_name, definite_path.string());

    if (!std::filesystem::exists(definite_path)) {
      std::filesystem::create_directory(definite_path);
    }
  } catch (std::exception& e) {
    LOG(error, "Failed to create directory {}: {}", path.string(), e.what());
    exit(1);
  }
}

void VulcanParam::init(const char* prog_name) {
  process_name_ = process_name(prog_name);

  // Initialize runtime direcotries
  check_and_create_dir(VULCAN_HOME, get(VULCAN_HOME));
  check_and_create_dir(VULCAN_DATA_DIR, get(VULCAN_DATA_DIR));
  check_and_create_dir(VULCAN_LOG_DIR, get(VULCAN_LOG_DIR));
}

}  // namespace vulcan
