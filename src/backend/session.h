// Copyright 2023 VulcanDB
#pragma once

#include <string>

#include "backend/db.h"
#include "backend/session.h"

namespace vulcan {

class Session {
 public:
  // static Session &current();
  static Session &default_session();

 public:
  Session() = default;
  ~Session();

  Session(const Session &other);
  void operator=(Session &) = delete;

  const char *get_current_db_name() const;
  Db *get_current_db() const;

  void set_current_db(const std::string &dbname);

 private:
  Db *db_ = nullptr;
};

}  // namespace vulcan
