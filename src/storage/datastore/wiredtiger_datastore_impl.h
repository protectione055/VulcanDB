// Copyright 2023 VulcanDB

#pragma once

#include <wiredtiger.h>

#include <iostream>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "common/vulcan_logger.h"
#include "storage/datastore/datastore_interface.h"

namespace vulcan {

struct WiredTigerDataStoreConfig {
  std::string data_home_dir;
  std::string table_name;

  explicit WiredTigerDataStoreConfig(std::string _data_home_dir,
                                     std::string _table_name)
      : data_home_dir(std::move(_data_home_dir)),
        table_name(std::move(_table_name)) {}
};

class WiredTigerSession : public DataStoreSession {
 public:
  WiredTigerSession() = default;
  explicit WiredTigerSession(WT_CONNECTION* const conn, WT_SESSION* session,
                             std::string table_name);
  ~WiredTigerSession() = default;

  const char* get(const char* key) override;
  void upsert_kv(const char* key, const char* value) override;
  bool insert_kv(const char* key, const char* value) override;
  void delete_kv(const char* key) override;

 private:
  WT_CONNECTION* const conn_;
  WT_SESSION* session_;
  WT_CURSOR* cursor_;
  std::string table_name_;

  void CheckOp(const int ret, const char* funcname) {
    if (ret != 0) {
      VULCAN_LOG(error, "WiredTigerSession::CheckOp: {} failed. {}", funcname,
                 strerror(ret));
      exit(1);
    }
  }
};

class WiredTigerDataStore : public DataStoreInterface {
 public:
  WiredTigerDataStore() = default;
  ~WiredTigerDataStore() = default;

  int open_datastore_instance(void* datastore_config) override;
  std::shared_ptr<DataStoreSession> new_datastore_session() override;
  int close_datastore_instance() override;

  WT_CONNECTION* get_conn() { return conn_; }

 private:
  WT_CONNECTION* conn_;
  std::mutex mutex_;
  WiredTigerDataStoreConfig* config_ = nullptr;
  std::string table_name_;

  // TODO(Ziming Zhang): remove this
  void CheckOp(const int ret, const char* funcname) {
    if (ret != 0) {
      VULCAN_LOG(error, "WiredTigerDataStore::CheckOp: {} failed", funcname);
    }
  }
};

}  // namespace vulcan
