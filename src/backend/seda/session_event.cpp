// Copyright 2023 VulcanDB

#include "backend/seda/session_event.h"

namespace vulcan {

SessionEvent::SessionEvent(ConnectionContext *client) : client_(client) {}

SessionEvent::~SessionEvent() {}

ConnectionContext *SessionEvent::get_client() const { return client_; }

Session *SessionEvent::session() const { return client_->session; }

const char *SessionEvent::get_response() const { return response_.c_str(); }

void SessionEvent::set_response(const char *response) {
  set_response(response, strlen(response));
}

void SessionEvent::set_response(const char *response, int len) {
  response_.assign(response, len);
}

void SessionEvent::set_response(std::string &&response) {
  response_ = std::move(response);
}

int SessionEvent::get_response_len() const { return response_.size(); }

char *SessionEvent::get_request_buf() { return client_->buf; }

int SessionEvent::get_request_buf_len() { return SOCKET_BUFFER_SIZE; }

}  // namespace vulcan
