#include <fstream>
#include <iostream>
#include <string>

#include "DeflateTestRunner.h"
#include "yaml-cpp/yaml.h"

void clear_directory(const std::filesystem::path& path);

int main() {
  try {
    while (std::filesystem::current_path().filename() !=
           "ifc-compression-benchmark") {
      std::filesystem::current_path("..");
    }

    // 读取配置文件并进行初始化
    std::string config_file = "config.yaml";
    YAML::Node config = YAML::LoadFile(config_file);
    if (!config["global_config"].IsDefined()) {
      std::cerr << "[ERROR] \"global_config\" is not defined in config.yaml"
                << std::endl;
      exit(1);
    }

    YAML::Node global_config = config["global_config"];
    const std::string WORKING_DIR = global_config["result_dir"].Scalar();
    const std::string DATA_DIR = global_config["source_dir"].Scalar();

    if (!std::filesystem::exists(WORKING_DIR)) {
      std::filesystem::create_directory(WORKING_DIR);
    }
    clear_directory(WORKING_DIR);

    if (!std::filesystem::exists(DATA_DIR)) {
      std::cerr << "Error: " << DATA_DIR << " does not exist." << std::endl;
      exit(1);
    }

    // 执行zip算法压缩测试
    YAML::Node deflate_config = config["deflate_test_config"];
    YAML::Node compress_level_vector = deflate_config["compress_level"];
    YAML::Node chunk_size_vector = deflate_config["chunk_size"];
    std::vector<int> compress_levels;
    std::vector<long int> chunk_sizes;

    for (size_t i = 0; i < compress_level_vector.size(); ++i) {
      compress_levels.push_back(std::stoi(compress_level_vector[i].Scalar()));
    }

    for (size_t i = 0; i < chunk_size_vector.size(); ++i) {
      chunk_sizes.push_back(std::stol(chunk_size_vector[i].Scalar()));
    }

    compbench::DeflateTestRunner deflate_runner;
    deflate_runner.setup(WORKING_DIR, DATA_DIR);
    deflate_runner.set_compress_levels(compress_levels);
    deflate_runner.set_chunk_sizes(chunk_sizes);
    deflate_runner.run();
    deflate_runner.teardown();
  } catch (const std::exception& e) {
    std::cerr << "[Deflate Test Failed] " << e.what() << std::endl;
    exit(1);
  }

  std::cout << "All Test Runned Successfully! Weee!" << std::endl;
  return 0;
}

void clear_directory(const std::filesystem::path& path) {
  for (auto& entry : std::filesystem::directory_iterator(path)) {
    std::filesystem::remove_all(entry.path());
  }
}