#pragma once

#include <string>

namespace compbench {

class TestRunnerInterface {
 private:
  /* data */
 public:
  TestRunnerInterface() = default;
  ~TestRunnerInterface() = default;

  virtual void setup(std::string workding_dir, std::string data_dir) = 0;
  virtual void run() = 0;
  virtual void teardown() = 0;
};

}  // namespace compbench