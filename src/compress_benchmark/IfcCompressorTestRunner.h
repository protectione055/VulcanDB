// Copyright 2023 protectione055@foxmail.com

#pragma once

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "TestRunnerInterface.h"
#include "TestUtil.h"

namespace compbench {

class IfcCompressorTestRunner : public TestRunnerInterface {
 private:
  std::string input_dir_;
  std::string output_dir_;
  // FPR参数向量
  std::vector<double> fpr_vector_ = {0, 0.001, 0.005, 0.01, 0.05, 0.1, 0.5};

 public:
  IfcCompressorTestRunner() = default;
  ~IfcCompressorTestRunner() = default;

  void setup(std::string input_dir, std::string output_dir) override;
  void run() override;
  void teardown() override;

  void set_fpr_vector(const std::vector<double>& fpr_vector) {
    fpr_vector_ = fpr_vector;
  }
};

}  // namespace compbench
