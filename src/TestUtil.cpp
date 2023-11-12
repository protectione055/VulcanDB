#include "TestUtil.h"

#include <fstream>

namespace compbench {

void traverseDir(
    const std::filesystem::path& root_dir,
    const std::filesystem::path& output_dir, std::string origin_postfix,
    std::string target_postfix,
    std::function<void(const std::string&, const std::string&)> convert_func) {
  if (!std::filesystem::exists(root_dir)) {
    std::cerr << "Root directory does not exist" << std::endl;
    exit(1);
  }

  for (const auto& entry :
       std::filesystem::recursive_directory_iterator(root_dir)) {
    std::string relative_path =
        std::filesystem::relative(entry.path(), root_dir);
    //检查输出目标目录是否存在，不存在则创建
    if (!entry.is_regular_file()) {
      std::string target_dir =
          output_dir / std::filesystem::relative(entry.path(), root_dir);
      if (!std::filesystem::exists(target_dir)) {
        std::filesystem::create_directories(target_dir);
      }
      continue;
    }

    //处理输入文件
    std::string input_filename = entry.path().string();
    std::string output_filename = output_dir / relative_path;
    size_t replace_pos = output_filename.find(origin_postfix);
    if (replace_pos == std::string::npos) {
      output_filename += target_postfix;
    } else {
      output_filename.replace(replace_pos, origin_postfix.length(),
                              target_postfix);
    }
    convert_func(input_filename, output_filename);
  }
}

}  // namespace compbench