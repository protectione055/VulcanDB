// Copyright 2023 VulcanDB

#pragma once

#include <memory>
#include <string>

#include "ifcparse/IfcFile.h"

namespace vulcan {

class VulcanIfcModel {
 public:
  VulcanIfcModel() = delete;

  // 从指定路径加载ifc模型
  // @param ifc_file_path ifc文件路径
  // @throw std::runtime_error 文件被另一个进程占用
  explicit VulcanIfcModel(const std::string& ifc_file_path);
  ~VulcanIfcModel() = default;

  int get_max_id() const;

 private:
  VulcanIfcModel(const VulcanIfcModel&) = delete;

  std::string ifc_file_path_;
  std::shared_ptr<IfcParse::IfcFile> ifc_file_ = nullptr;
};

}  // namespace vulcan
