// Copyright 2023 protectione055@foxmail.com

#include "IfcCompressorTestRunner.h"

namespace compbench {

void IfcCompressorTestRunner::setup(std::string input_dir,
                                    std::string output_dir) {
  input_dir_ = input_dir;
  output_dir_ = std::filesystem::path(output_dir) / "IfcCompressor";
  if (!std::filesystem::exists(input_dir_)) {
    std::cerr << "Input directory does not exist" << std::endl;
    exit(1);
  }

  if (!std::filesystem::exists(output_dir_)) {
    std::filesystem::create_directories(output_dir_);
    std::cout << output_dir_ << " not found. Create it." << std::endl;
  }
}

void IfcCompressorTestRunner::run() {
  std::cout << "IfcCompressor Test is running..." << std::endl;

  IfcCompressorConverter ifc_compressor;
  for (float fpr : fpr_vector_) {
    ifc_compressor.set_fpr(fpr);
    std::filesystem::path output_dir =
        std::filesystem::path(output_dir_) / ifc_compressor.method_name();
    if (!std::filesystem::exists(output_dir)) {
      std::filesystem::create_directories(output_dir);
      std::cout << output_dir << " not found. Create it." << std::endl;
    }
    traverseDir(input_dir_, output_dir, ".ifc", ".ifc", ifc_compressor);
  }
}

void IfcCompressorTestRunner::teardown() {
  std::cout << "IfcCompressorTestRunner run successfully!" << std::endl;
}

}  // namespace compbench
