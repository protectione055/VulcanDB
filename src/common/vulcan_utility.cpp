// Copyright 2023 Ziming Zhang

#include "common/vulcan_utility.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace vulcan {

std::filesystem::path create_temp_directory() {
  std::string temp_dir_template =
      std::filesystem::temp_directory_path().string() + "/my_temp_dir_XXXXXX";
  char* temp_dir_name = mkdtemp(const_cast<char*>(temp_dir_template.c_str()));
  if (temp_dir_name == nullptr) {
    throw std::runtime_error("Failed to create temporary directory");
  }
  return std::filesystem::path(temp_dir_name);
}

void replace_win_newlines_to_unix(std::string file, std::string new_file) {
  try {
    new_file = new_file == "" ? file : new_file;
    std::filesystem::path file_path(file);
    std::fstream fs(file_path, std::ios::in | std::ios::out);
    std::filesystem::path tmp_file =
        create_temp_directory() / file_path.filename();
    std::fstream tmp_fs(tmp_file, std::ios::out);
    std::string line;

    while (std::getline(fs, line)) {
      line = subreplace(line, "\r", "");
      tmp_fs << line << std::endl;
    }

    // std::filesystem::remove(file_path);
    std::filesystem::remove(file);
    std::filesystem::copy(tmp_file, new_file);
    std::filesystem::remove(tmp_file);

  } catch (std::exception& e) {
    std::cout << e.what() << std::endl;
  }
}

std::string subreplace(std::string resource_str, std::string sub_str,
                       std::string new_str) {
  std::string dst_str = resource_str;
  std::string::size_type pos = 0;
  while ((pos = dst_str.find(sub_str)) != std::string::npos) {
    dst_str.replace(pos, sub_str.length(), new_str);
  }
  return dst_str;
}

}  // namespace vulcan
