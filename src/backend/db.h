// Copyright 2023 VulcanDB
#pragma once

#include <string>

namespace vulcan {

class Db {
 public:
  Db() = default;
  ~Db() = default;

  // 初始化数据库
  // @param name 数据库名称
  // @param dbpath 数据库路径
  // @throw 如果数据库初始化失败，抛出异常
  void init(const char *name, const char *dbpath);

  const char *name() const { return name_.c_str(); }
  const char *path() const { return path_.c_str(); }

 private:
  std::string name_;
  std::string path_;
};  // namespace vulcan

}  // namespace vulcan
