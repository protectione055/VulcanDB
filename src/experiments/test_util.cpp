// Copyright 2023 VulcanDB
#include "test_util.h"

#include <fstream>

namespace compbench {

std::string TestWatcher::get_header() const {
  std::string header =
      "experiment_name,origin_size,compress_size,compress_ratio,compress_"
      "time,compress_speed,memory_usage";
  return header;
}

double TestWatcher::compress_ratio() const {
  if (compress_size_ == INVALID_VALUE || origin_size_ == INVALID_VALUE) {
    return INVALID_VALUE;
  }
  return compress_size_ / origin_size_;
}

double TestWatcher::compress_speed() const {
  if (compress_time_ == INVALID_VALUE || origin_size_ == INVALID_VALUE) {
    return INVALID_VALUE;
  }
  return origin_size_ / compress_time_;
}

std::string TestWatcher::get_result() const {
  std::string result = experiment_name_ + ",";
  result += std::to_string(origin_size_) + " ,";
  result += std::to_string(compress_size_) + " ,";
  result += std::to_string(compress_ratio()) + " ,";
  result += std::to_string(compress_time_) + " ,";
  result += std::to_string(compress_speed()) + " ,";
  result += std::to_string(memory_usage_) + " ,";
  return result;
}

// 遍历目录，对目录下的所有文件执行转换
void traverseDir(const std::filesystem::path& root_dir,
                 const std::filesystem::path& output_dir,
                 std::string origin_postfix, std::string target_postfix,
                 Converter& converter) {
  if (!std::filesystem::exists(root_dir)) {
    std::cerr << "Root directory does not exist" << std::endl;
    exit(1);
  }

  // 初始化结果输出文件
  try {
    compbench::TestWatcher test_watcher;
    std::ofstream profile_file(output_dir.parent_path() / "profile.csv",
                               std::ios::app);
    profile_file << test_watcher.get_header() << std::endl;

    for (const auto& entry :
         std::filesystem::recursive_directory_iterator(root_dir)) {
      std::string relative_path =
          std::filesystem::relative(entry.path(), root_dir);
      // 转换前初始化
      if (!entry.is_regular_file()) {
        std::string target_dir =
            output_dir / std::filesystem::relative(entry.path(), root_dir);
        if (!std::filesystem::exists(target_dir)) {
          std::filesystem::create_directories(target_dir);
        }
        continue;
      }

      if (entry.path().extension() != origin_postfix) {
        continue;
      }

      std::string input_filename = entry.path().string();
      std::string output_filename = output_dir / relative_path;
      size_t replace_pos = output_filename.find(origin_postfix);
      if (replace_pos == std::string::npos) {
        output_filename += target_postfix;
      } else {
        output_filename.replace(replace_pos, origin_postfix.length(),
                                target_postfix);
      }

      // 执行测试
      test_watcher.set_experiment_name(output_filename);
      test_watcher.start();
      converter.convert(input_filename, output_filename);
      test_watcher.end();
      test_watcher.set_origin_size(std::filesystem::file_size(input_filename));
      test_watcher.set_compress_size(
          std::filesystem::file_size(output_filename));
      profile_file << test_watcher.get_result() << std::endl;
    }
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    exit(1);
  }
}

}  // namespace compbench
