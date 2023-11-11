#include <fstream>
#include <iostream>
#include <string>

#include "ZipTestRunner.h"

int main() {
  const std::string WORKING_DIR = "../resources/result_files";
  const std::string DATA_DIR = "../resources/source_files";

  if (!std::filesystem::exists(WORKING_DIR)) {
    std::filesystem::create_directory(WORKING_DIR);
  }

  if (!std::filesystem::exists(DATA_DIR)) {
    std::cerr << "Error: " << DATA_DIR << " does not exist." << std::endl;
    exit(1);
  }

  compbench::ZipTestRunner zip_runner;
  zip_runner.setup(WORKING_DIR, DATA_DIR);
  zip_runner.run();
  zip_runner.teardown();

  return 0;
}