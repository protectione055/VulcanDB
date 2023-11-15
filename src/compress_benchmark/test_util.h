// Copyright 2023 protectione055@foxmail.com
#pragma once

#include <chrono>
#include <filesystem>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace compbench {

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
  explicit TestWatcher(std::string name) : experiment_name_(name) {}
  ~TestWatcher() = default;

  void start() { timer_.reset(); }
  void end() { compress_time_ = timer_.elapsed(); }
  // 获取当前任务的表头
  std::string get_header() const;

  // 获取上一个任务的结果，以","分割的csv格式返回
  std::string get_result() const;
  // 获取实验名称
  void set_experiment_name(std::string name) { experiment_name_ = name; }
  // 记录原始文件大小
  void set_origin_size(double size) { origin_size_ = size; }
  // 记录压缩后文件大小
  void set_compress_size(double size) { compress_size_ = size; }
  // 计算压缩时间
  double compress_ratio() const;
  // 计算压缩速度
  double compress_speed() const;

  // TODO(Ziming Zhang): 获取运行期间的平均内存占用

 public:
  std::string experiment_name_;
  double origin_size_ = INVALID_VALUE;
  double compress_size_ = INVALID_VALUE;
  double compress_time_ = INVALID_VALUE;
  double memory_usage_ = INVALID_VALUE;

 private:
  class Timer timer_;
};

class Converter {
 public:
  Converter() = default;
  ~Converter() = default;

  virtual void convert(const std::string& input_file,
                       const std::string& output_file) = 0;
  virtual std::string method_name() const = 0;
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

}  // namespace compbench
