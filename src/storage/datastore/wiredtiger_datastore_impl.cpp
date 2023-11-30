// Copyright 2023 VulcanDB

#include "storage/datastore/wiredtiger_datastore_impl.h"

#include <string>

namespace vulcan {

int WiredTigerDataStore::open_datastore_instance(void* datastore_config) {
  config_ = reinterpret_cast<WiredTigerDataStoreConfig*>(datastore_config);
  if (!std::filesystem::exists(config_->data_home_dir)) {
    std::filesystem::create_directories(config_->data_home_dir);
  }

  table_name_ = "table:" + config_->table_name;

  CheckOp(wiredtiger_open(config_->data_home_dir.c_str(), nullptr, "create",
                          &conn_),
          "wiredtiger_open");

  return 0;
}

WiredTigerSession::WiredTigerSession(WT_CONNECTION* const conn,
                                     WT_SESSION* session,
                                     std::string table_name)
    : conn_(conn), session_(session) {
  table_name_ = table_name;
  CheckOp(session_->create(session_, table_name_.c_str(),
                           "key_format=S,value_format=S"),
          "session_->create");
  CheckOp(session_->open_cursor(session_, table_name_.c_str(), nullptr, nullptr,
                                &cursor_),
          "session_->open_cursor");
}

/**
 * @brief Opens a session in the WiredTiger data store.
 *
 * This function opens a session in the WiredTiger data store. If the connection
 * is null, it throws a runtime_error. Otherwise, it opens a session using the
 * conn_ object and returns a pointer to the newly created WiredTigerSession
 * object.
 *
 * @return WiredTigerSession* A pointer to the newly created WiredTigerSession
 * object.
 * @throws std::runtime_error if the datastore is not inistialized(conn_ is
 * null).
 */
std::shared_ptr<DataStoreSession> WiredTigerDataStore::new_datastore_session() {
  WT_SESSION* session;
  if (conn_ == nullptr) {
    LOG(error, "WiredTigerDataStore::new_datastore_session: conn_ is null");
    throw std::runtime_error(
        "WiredTigerDataStore::new_datastore_session: conn_ is null");
  }

  CheckOp(conn_->open_session(conn_, nullptr, nullptr, &session),
          "conn_->open_session");
  return std::make_shared<WiredTigerSession>(conn_, session, table_name_);
}

int WiredTigerDataStore::close_datastore_instance() {
  return conn_->close(conn_, nullptr);
}

/**
 * Retrieves the value associated with the specified key from the WiredTiger
 * datastore.
 *
 * @param key The key to retrieve the value for.
 * @return The value associated with the key, or nullptr if the key does not
 * exist.
 */
const char* WiredTigerSession::get(const char* key) {
  if (cursor_ == nullptr) {
    CheckOp(session_->open_cursor(session_, table_name_.c_str(), nullptr,
                                  nullptr, &cursor_),
            "session_->open_cursor");
  }
  cursor_->set_key(cursor_, key);
  int ret = cursor_->search(cursor_);
  if (ret == 0) {
    const char* value;
    CheckOp(cursor_->get_value(cursor_, &value), "cursor_->get_value");
    return value;
  }
  return nullptr;
}

/**
 * Upserts a key-value pair into the WiredTiger datastore.
 * If the key already exists, the value will be updated; otherwise, a new
 * key-value pair will be inserted.
 *
 * @param key The key to be upserted.
 * @param value The value to be upserted.
 */
void WiredTigerSession::upsert_kv(const char* key, const char* value) {
  if (cursor_ == nullptr) {
    CheckOp(session_->open_cursor(session_, table_name_.c_str(), nullptr,
                                  nullptr, &cursor_),
            "session_->open_cursor");
  }
  cursor_->set_key(cursor_, key);
  cursor_->set_value(cursor_, value);
  CheckOp(cursor_->insert(cursor_), "cursor_->insert");
}

/**
 * Inserts a key-value pair into the WiredTiger datastore.
 *
 * @param key The key to be inserted.
 * @param value The value to be inserted.
 * @return True if the key-value pair was successfully inserted, false if the
 * key already exists.
 */
bool WiredTigerSession::insert_kv(const char* key, const char* value) {
  if (cursor_ == nullptr) {
    CheckOp(session_->open_cursor(session_, table_name_.c_str(), nullptr,
                                  nullptr, &cursor_),
            "session_->open_cursor");
  }
  cursor_->set_key(cursor_, key);
  if (cursor_->search(cursor_) == 0) {
    // Key already exists, do nothing
    return false;
  }
  cursor_->set_value(cursor_, value);
  CheckOp(cursor_->insert(cursor_), "cursor_->insert");

  return true;
}

/**
 * @brief Deletes a key-value pair from the WiredTiger datastore.
 *
 * @param key The key to delete.
 */
void WiredTigerSession::delete_kv(const char* key) {
  if (cursor_ == nullptr) {
    CheckOp(session_->open_cursor(session_, table_name_.c_str(), nullptr,
                                  nullptr, &cursor_),
            "session_->open_cursor");
  }
  cursor_->set_key(cursor_, key);
  CheckOp(cursor_->remove(cursor_), "cursor_->remove");
}

}  // namespace vulcan
