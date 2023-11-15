// Copyright 2023 protectione055@foxmail.com

#pragma once

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "compress_benchmark/test_runner_interface.h"
#include "compress_benchmark/test_util.h"
#include "storage/compression/ifccompressor_impl.h"
#include "common/vulcan_logger.h"

namespace compbench {

class IfcCompressorConverter : public compbench::Converter {
 private:
  vulcan::IfcCompressorImpl ifc_compressor_;
  vulcan::IfcCompressorArg ifc_compressor_arg_;

 public:
  IfcCompressorConverter() = default;
  ~IfcCompressorConverter() = default;

  void convert(const std::string& input_file,
               const std::string& output_file) override {
    ifc_compressor_.compress(input_file, output_file, &ifc_compressor_arg_);
  }

  std::string method_name() const override {
    return "IfcCompressor-" + std::to_string(ifc_compressor_arg_.fpr);
  }

  void set_fpr(const float fpr) { ifc_compressor_arg_.fpr = fpr; }
};

class IfcCompressorTestRunner : public TestRunnerInterface {
 private:
  std::string input_dir_;
  std::string output_dir_;
  // FPR参数向量
  std::vector<float> fpr_vector_ = {0, 0.001, 0.005, 0.01, 0.05, 0.1, 0.5};

 public:
  IfcCompressorTestRunner() = default;
  ~IfcCompressorTestRunner() = default;

  void setup(std::string input_dir, std::string output_dir) override;
  void run() override;
  void teardown() override;

  void set_fpr_vector(const std::vector<float>& fpr_vector) {
    fpr_vector_ = fpr_vector;
  }
};

}  // namespace compbench
