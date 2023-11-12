#pragma once

#include <filesystem>
#include <fstream>
#include <functional>
#include <string>

#include "TestRunnerInterface.h"
#include "zlib.h"

namespace compbench {

class Zip {
 public:
  const static long int CHUNCK_SIZE = 16384;

  /*
   * 将文件输入流压缩为ZIP格式的输出流
   *
   * @param ifs 输入文件流
   * @param level
   * 压缩等级(压缩等级取值1-9，压缩率和压缩时间递增；0代表不进行压缩)
   * @return std::iostream& 压缩结果的输出流
   */
  static void compress(const std::string& input_filename,
                       const std::string& output_filename,
                       int level = Z_DEFAULT_COMPRESSION,
                       long int chunck_size = CHUNCK_SIZE);

  static void decompress(const std::string& input_filename,
                         const std::string& output_filename);
};

// 负责测试Zip压缩算法
class ZipTestRunner : public TestRunnerInterface {
 public:
  ZipTestRunner() = default;
  ~ZipTestRunner() = default;

  void setup(std::string workding_dir, std::string data_dir);
  void run();
  void teardown();

  // 设置实验测试的压缩级别向量
  void set_compress_levels(const std::vector<int>& compress_levels) {
    if (compress_levels.empty()) {
      return;
    }
    compress_levels_ = compress_levels;
  }

  // 设置实验测试的块大小向量
  void set_chunk_sizes(const std::vector<long int>& chunk_sizes) {
    if (chunk_sizes.empty()) {
      return;
    }
    chunk_sizes_ = chunk_sizes;
  }

 private:
  std::filesystem::path working_dir_;
  std::filesystem::path data_dir_;
  std::vector<int> compress_levels_ = {Z_DEFAULT_COMPRESSION};
  std::vector<long int> chunk_sizes_ = {16384};

  // 从输入文件流读入数据，通过zip压缩后写入输出流
  struct ZipTask {
    int level = Z_DEFAULT_COMPRESSION;
    long int chunck_size = 16384;

    ZipTask(int level = Z_DEFAULT_COMPRESSION, long int chunck_size = 16384)
        : level(level), chunck_size(chunck_size) {}

    void operator()(const std::string& input_filename,
                    const std::string& output_filename) {
      Zip::compress(input_filename, output_filename, level, chunck_size);
    }
  };
};

}  // namespace compbench