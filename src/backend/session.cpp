// Copyright 2023 VulcanDB
#include "backend/session.h"

#include "backend/db.h"
#include "common/vulcan_logger.h"

namespace vulcan {

Session &Session::default_session() {
  static Session session;
  return session;
}

Session::Session(const Session &other) : db_(other.db_) {}

Session::~Session() {}

const char *Session::get_current_db_name() const {
  if (db_ != nullptr)
    return db_->name();
  else
    return "";
}

Db *Session::get_current_db() const { return db_; }

void Session::set_current_db(const std::string &dbname) {
  // TODO(Ziming Zhang): implement this function
}

}  // namespace vulcan
