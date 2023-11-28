// Copyright 2023 VulcanDB
#pragma once

#include <memory>
#include <string>

#include "common/vulcan_logger.h"

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

class DataStoreSession {
 public:
  DataStoreSession() = default;
  ~DataStoreSession() = default;

  // 底层存储引擎访问接口
  virtual const char* get(const char* key) = 0;
  virtual void upsert_kv(const char* key, const char* value) = 0;
  virtual bool insert_kv(const char* key, const char* value) = 0;
  virtual void delete_kv(const char* key) = 0;
  // TODO: Scan接口
};

// 底层存储引擎的抽象接口
class DataStoreInterface {
 public:
  DataStoreInterface() = default;
  virtual ~DataStoreInterface() = default;

  // 打开一个存储系统实例
  virtual int open_datastore_instance(void* datastore_config) = 0;
  virtual int close_datastore_instance() = 0;

  virtual std::shared_ptr<DataStoreSession> new_datastore_session() = 0;
};

class DataStoreFactory {
 public:
  DataStoreFactory() = default;
  ~DataStoreFactory() = default;

  // 创建一个存储引擎实例
  virtual std::shared_ptr<DataStoreInterface> create_datastore_instance(
      const DataStoreParameters& datastore_parameters) = 0;
};

}  // namespace vulcan
