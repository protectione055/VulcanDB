// Copyright 2023 VulcanDB
#pragma once

#include <string>

#include "backend/db.h"
#include "backend/session.h"
#include "common/defs.h"
#include "libevent/include/event.h"

namespace vulcan {

/**
 * @brief Represents a session in the VulcanDB backend.
 *
 * A session is responsible for managing the current database and providing
 * access to its functionality.
 */
class Session {
 public:
  // static Session &current();
  static Session &default_session();

 public:
  Session() = default;
  ~Session();

  Session(const Session &other);
  void operator=(Session &) = delete;

  /**
   * @brief Get the name of the current database.
   *
   * @return const char* The name of the current database.
   */
  const char *get_current_db_name() const;

  /**
   * @brief Get a pointer to the current database.
   *
   * @return Db* A pointer to the current database.
   */
  Db *get_current_db() const;

  /**
   * @brief Set the current database.
   *
   * @param dbname The name of the database to set as the current database.
   */
  void set_current_db(const std::string &dbname);

 private:
  Db *db_ = nullptr;
};

/**
 * @brief Represents the context of a connection.
 *
 * This struct holds information related to a connection, including the
 * associated session, file descriptor, read event, mutex, address, and buffer.
 */
typedef struct _ConnectionContext {
  Session *session;             /* Pointer to the associated session */
  int fd;                       /* File descriptor of the connection */
  struct event read_event;      /* Read event for the connection */
  pthread_mutex_t mutex;        /* Mutex for thread synchronization */
  char addr[24];                /* Address of the connection */
  char buf[SOCKET_BUFFER_SIZE]; /* Buffer for storing data */
} ConnectionContext;

}  // namespace vulcan
