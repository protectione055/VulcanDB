#pragma once

#include <chrono>
#include <filesystem>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace testutil {

class Converter;

// 计时器
class Timer {
 public:
  Timer() : start_(std::chrono::high_resolution_clock::now()) {}

  void reset() { start_ = std::chrono::high_resolution_clock::now(); }

  double elapsed() const {
    return std::chrono::duration_cast<std::chrono::duration<double>>(
               std::chrono::high_resolution_clock::now() - start_)
        .count();
  }

 private:
  std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

// 测试结果的包装类
class TestWatcher {
  friend void traverseDir(const std::filesystem::path& root_dir,
                          const std::filesystem::path& output_dir,
                          std::string origin_postfix,
                          std::string target_postfix, Converter& converter);

 public:
  const double INVALID_VALUE = -1;

  TestWatcher() = default;
  TestWatcher(std::string name) : experiment_name_(name) {}
  ~TestWatcher() = default;

  void start() { timer_.reset(); }
  void end() { compress_time_ = timer_.elapsed(); }

  std::string get_header() const {
    std::string header =
        "experiment_name,origin_size,compress_size,compress_ratio,compress_"
        "time,compress_speed,memory_usage";
    return header;
  }

  std::string get_result() const {
    std::string result = experiment_name_ + ",";
    result += std::to_string(origin_size_) + " ,";
    result += std::to_string(compress_size_) + " ,";
    result += std::to_string(compress_ratio()) + " ,";
    result += std::to_string(compress_time_) + " ,";
    result += std::to_string(compress_speed()) + " ,";
    result += std::to_string(memory_usage_) + " ,";
    return result;
  }

  void set_experiment_name(std::string name) { experiment_name_ = name; }
  void set_origin_size(double size) { origin_size_ = size; }
  void set_compress_size(double size) { compress_size_ = size; }

  double compress_ratio() const {
    if (compress_size_ == INVALID_VALUE || origin_size_ == INVALID_VALUE) {
      return INVALID_VALUE;
    }
    return compress_size_ / origin_size_;
  }

  double compress_speed() const {
    if (compress_time_ == INVALID_VALUE || origin_size_ == INVALID_VALUE) {
      return INVALID_VALUE;
    }
    return origin_size_ / compress_time_;
  }

  // TODO: 获取运行期间的平均内存占用

 public:
  std::string experiment_name_;
  double origin_size_ = INVALID_VALUE;
  double compress_size_ = INVALID_VALUE;
  double compress_time_ = INVALID_VALUE;
  double memory_usage_ = INVALID_VALUE;

 private:
  class Timer timer_;
};  // namespace testutil

class Converter {
 public:
  Converter() = default;
  ~Converter() = default;

  virtual void convert(const std::string& input_file,
                       const std::string& output_file) = 0;
  virtual std::string method_name() = 0;
};

/*接受一个转换函数作为参数，递归遍历目录的每个文件，对每个输入流执行该函数并转换为一个输出流
 *
 * @param root_dir 遍历的根目录
 * @param output_dir 输出目录
 * @param origin_postfix 原始文件后缀
 * @param target_postfix 目标文件后缀
 * @param config 传递给convert_func的参数
 * @param convert_func
 * 对文件的处理函数，处理后的文件输出到output_dir，返回结果的iostream
 */
void traverseDir(const std::filesystem::path& root_dir,
                 const std::filesystem::path& output_dir,
                 std::string origin_postfix, std::string target_postfix,
                 Converter& converter);

}  // namespace testutil