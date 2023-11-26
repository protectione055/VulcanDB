// Copyright 2023 VulcanDB
#include "common/io/resp_protocol.h"

#include <string>

namespace vulcan {

std::string RespProtocol::encode(const std::string &str) {
  std::string result;
  result.reserve(str.size() + 2);
  result.push_back('+');
  result.append(str);
  result.append("\r\n");
  return result;
}

std::string RespProtocol::decode(const std::string &str) {
  std::string result;
  result.reserve(str.size() - 2);
  result.append(str, 1, str.size() - 3);
  return result;
}

}  // namespace vulcan
