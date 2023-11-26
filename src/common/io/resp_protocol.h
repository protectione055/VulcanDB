// Copyright 2023 VulcanDB
#pragma once

#include <string>

namespace vulcan {

// RESP protocol实现
class RespProtocol {
 public:
  static std::string encode(const std::string &str);
  static std::string decode(const std::string &str);

 private:
};

}  // namespace vulcan
