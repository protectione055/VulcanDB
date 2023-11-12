#include "TestUtil.h"

#include <fstream>

namespace testutil {

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
    testutil::TestWatcher test_watcher;
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

}  // namespace testutil