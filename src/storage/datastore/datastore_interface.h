// Copyright 2023 VulcanDB
#pragma once

#include <common/vulcan_logger.h>

#include <memory>
#include <string>

namespace vulcan {

class VulcanKey {
 public:
  VulcanKey() = default;
  ~VulcanKey() = default;

 private:
  std::string key_;
};

template <typename T>
class VulcanValue {
 public:
  VulcanValue() = default;
  explicit VulcanValue(const T& value) : value_(value) {}
  ~VulcanValue() = default;

  T get_value() const { return value_; }
  void set_value(const T& value) { value_ = value; }

 private:
  T value_;
};

// 存储引擎参数
class DataStoreParameters {
 public:
  DataStoreParameters() = default;
  ~DataStoreParameters() = default;

  virtual std::string get_datastore_type() const = 0;
};

// 底层存储引擎的抽象接口
class DataStoreInterface {
 public:
  DataStoreInterface() = default;
  explicit DataStoreInterface(std::shared_ptr<VulcanLogger> logger) {}
  ~DataStoreInterface() = default;

  // 打开一个存储系统实例
  // @param 数据目录

  virtual void open_datastore_instance(const std::string& data_home_dir) = 0;
  // TODO: 完善接口
  //   virtual void new_datastore_session() = 0;

  // 底层存储引擎访问接口
  virtual const char* get(const char* key) = 0;
  virtual void upsert_kv(const char* key, const char* value) = 0;
  virtual void insert_kv(const char* key, const char* value) = 0;
  virtual void delete_kv(const char* key) = 0;
  // TODO: Scan接口
};

}  // namespace vulcan
