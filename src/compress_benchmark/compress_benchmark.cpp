// Copyright 2023 Ziming Zhang
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <string>

#include "common/vulcan_utility.h"
#include "deflate_test_runner.h"
#include "execution/ifc_spc_loader.h"
#include "ifccompressor_test_runner.h"
#include "yaml-cpp/yaml.h"

void load_config(const std::string& config_file);
void clear_directory(const std::filesystem::path& path);
void run_deflate_test();
void run_ifccompressor_test();

// 配置文件路径
std::string g_config_file =
    "/home/zzm/projects/ifc-compression-benchmark/configs/"
    "benchmark_config.yaml";
YAML::Node g_config;
// 工作目录
std::string g_working_dir;
// 数据目录
std::string g_data_dir;

int main(int argc, char** argv) {
  int para;
  while ((para = getopt(argc, argv, "c::")) != -1) {
    switch (para) {
      case 'c':
        g_config_file = optarg;
        break;
    }
  }

  load_config(g_config_file);

  //   run_deflate_test();
  run_ifccompressor_test();

  std::cout << "All Test Runned Successfully! Weee!" << std::endl;
  return 0;
}

// 执行deflate算法压缩测试
void run_deflate_test() {
  try {
    YAML::Node deflate_config = g_config["deflate_test_config"];
    YAML::Node compress_level_vector = deflate_config["compress_level"];
    YAML::Node chunk_size_vector = deflate_config["chunk_size"];
    std::vector<int> compress_levels;
    std::vector<int64_t> chunk_sizes;

    for (size_t i = 0; i < compress_level_vector.size(); ++i) {
      compress_levels.push_back(std::stoi(compress_level_vector[i].Scalar()));
    }

    for (size_t i = 0; i < chunk_size_vector.size(); ++i) {
      chunk_sizes.push_back(std::stol(chunk_size_vector[i].Scalar()));
    }

    compbench::DeflateTestRunner deflate_runner;
    deflate_runner.setup(g_working_dir, g_data_dir);
    deflate_runner.set_compress_levels(compress_levels);
    deflate_runner.set_chunk_sizes(chunk_sizes);
    deflate_runner.run();
    deflate_runner.teardown();
  } catch (const std::exception& e) {
    std::cerr << "[Deflate Test Failed] " << e.what() << std::endl;
    exit(1);
  }
}

// 执行ifc-compressor算法压缩测试
void run_ifccompressor_test() {
  try {
    YAML::Node ifccompressor_config = g_config["ifccompressor_test_config"];
    YAML::Node fpr_node = ifccompressor_config["fpr"];
    std::vector<float> fpr_vector;

    for (size_t i = 0; i < fpr_node.size(); ++i) {
      fpr_vector.push_back(std::stoi(fpr_node[i].Scalar()));
    }

    compbench::IfcCompressorTestRunner ifc_compressor_runner;
    ifc_compressor_runner.set_fpr_vector(fpr_vector);
    ifc_compressor_runner.setup(g_data_dir, g_working_dir);
    ifc_compressor_runner.run();
    ifc_compressor_runner.teardown();
  } catch (const std::exception& e) {
    std::cerr << "[IfcCompressor Test Failed] " << e.what() << std::endl;
    exit(1);
  }
}

void load_config(const std::string& config_file) {
  try {
    g_config = YAML::LoadFile(config_file);
    if (!g_config["global_config"].IsDefined()) {
      std::cerr << "[ERROR] \"global_config\" is not defined in config.yaml"
                << std::endl;
      exit(1);
    }

    YAML::Node global_config = g_config["global_config"];
    g_working_dir = global_config["result_dir"].Scalar();
    g_data_dir = global_config["source_dir"].Scalar();

    if (!std::filesystem::exists(g_working_dir)) {
      std::filesystem::create_directory(g_working_dir);
    }
    clear_directory(g_working_dir);

    if (!std::filesystem::exists(g_data_dir)) {
      std::cerr << "Error: " << g_data_dir << " does not exist." << std::endl;
      exit(1);
    }

    // 对目录下的文件进行预处理
    for (auto& entry :
         std::filesystem::recursive_directory_iterator(g_data_dir)) {
      if (entry.path().extension() == ".ifc") {
        vulcan::replace_win_newlines_to_unix(entry.path());
      }
    }
  } catch (const std::exception& e) {
    std::cerr << "[ERROR] " << e.what() << std::endl;
    exit(1);
  }
}

void clear_directory(const std::filesystem::path& path) {
  for (auto& entry : std::filesystem::directory_iterator(path)) {
    std::filesystem::remove_all(entry.path());
  }
}
