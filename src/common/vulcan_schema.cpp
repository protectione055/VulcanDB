// Copyright 2023 VulcanDB

#include "common/vulcan_schema.h"

#include <fcntl.h>

#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "common/vulcan_logger.h"
#include "common/vulcan_utility.h"

namespace vulcan {

VulcanIfcModel::VulcanIfcModel(const std::string& ifc_file_path)
    : ifc_file_path_(ifc_file_path) {
  try {
    // TODO: 在文件包装类上创建fstream
    // int fd = open_file_and_lock(ifc_file_path_, O_RDONLY);
    // if (fd == -1) {
    //   throw std::runtime_error(ifc_file_path_ +
    //                            " is being used by another process");
    // }

    // std::ifstream ifc_file_stream(ifc_file_path);
    // ifc_file_ = std::make_shared<IfcParse::IfcFile>(ifc_file_stream);
  } catch (const std::exception& e) {
    throw e;
  }
}

int VulcanIfcModel::get_max_id() const { return ifc_file_->getMaxId(); }

}  // namespace vulcan
