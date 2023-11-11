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

class ZipTestRunner : public TestRunnerInterface {
 public:
  ZipTestRunner() = default;
  ~ZipTestRunner() = default;

  void setup(std::string workding_dir, std::string data_dir);
  void run();
  void teardown();

 private:
  std::filesystem::path working_dir_;
  std::filesystem::path data_dir_;
};

}  // namespace compbench