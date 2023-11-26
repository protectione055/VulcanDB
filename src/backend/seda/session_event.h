// Copyright 2023 VulcanDB
#pragma once

#include <list>
#include <string>
#include <utility>

#include "backend/seda/stage_event.h"
#include "backend/session.h"

namespace vulcan {

class Session;

class SessionEvent : public StageEvent {
 public:
  explicit SessionEvent(ConnectionContext *client);
  virtual ~SessionEvent();

  ConnectionContext *get_client() const;
  Session *session() const;

  const char *get_response() const;
  void set_response(const char *response);
  void set_response(const char *response, int len);
  void set_response(std::string &&response);
  int get_response_len() const;
  char *get_request_buf();
  int get_request_buf_len();

 private:
  ConnectionContext *client_;

  std::string response_;
};

}  // namespace vulcan
