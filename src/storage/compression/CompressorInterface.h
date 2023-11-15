#pragma once

#include <string>

namespace vulcan {
class CompressorInterface {
 public:
  virtual void compress(const std::string& input_filename,
                        const std::string& output_filename,
                        const void* comp_arg) = 0;
  virtual void decompress(const std::string& input_filename,
                          const std::string& output_filename) = 0;
};
}  // namespace vulcan