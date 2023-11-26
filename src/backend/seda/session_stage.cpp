// Copyright 2023 VulcanDB
#include "backend/seda/session_stage.h"

#include <list>
#include <string>

#include "backend/seda/callback.h"
#include "backend/seda/session_event.h"
#include "backend/server.h"
#include "common/string.h"
#include "common/vulcan_logger.h"
// TODO(Ziming zhang): 引入对应的头文件
// #include "backend/seda/parse_stage.h"
// #include "backend/seda/sql_stage_event.h"

namespace vulcan {

// Constructor
SessionStage::SessionStage(const char *tag)
    : Stage(tag), plan_cache_stage_(nullptr) {}

// Destructor
SessionStage::~SessionStage() {}

// Parse properties, instantiate a stage object
Stage *SessionStage::make_stage(const std::string &tag) {
  SessionStage *stage = new (std::nothrow) SessionStage(tag.c_str());
  if (stage == nullptr) {
    VULCAN_LOG(error, "new ExecutorStage failed");
    return nullptr;
  }
  stage->set_properties();
  return stage;
}

// Set properties for this object set in stage specific properties
bool SessionStage::set_properties() {
  //  std::string stageNameStr(stage_name_);
  //  std::map<std::string, std::string> section = g_properties()->get(
  //    stageNameStr);
  //
  //  std::map<std::string, std::string>::iterator it;
  //
  //  std::string key;

  return true;
}

// Initialize stage params and validate outputs
bool SessionStage::initialize() {
  VULCAN_LOG(trace, "SessionStage initializing");

  std::list<Stage *>::iterator stgp = next_stage_list_.begin();
  plan_cache_stage_ = *(stgp++);

  VULCAN_LOG(trace, "SessionStage initialized successfully");
  return true;
}

// Cleanup after disconnection
void SessionStage::cleanup() {
  VULCAN_LOG(trace, "SessionStage cleaning up...");

  VULCAN_LOG(trace, "SessionStage cleanup successfully");
}

void SessionStage::handle_event(StageEvent *event) {
  VULCAN_LOG(trace, "SessionStage begin to handle event");

  // right now, we just support only one event.
  handle_request(event);

  VULCAN_LOG(trace, "SessionStage finish handling event");
  return;
}

void SessionStage::callback_event(StageEvent *event, CallbackContext *context) {
  VULCAN_LOG(trace, "Enter\n");

  SessionEvent *sev = dynamic_cast<SessionEvent *>(event);
  if (nullptr == sev) {
    VULCAN_LOG(error, "Cannot cat event to sessionEvent");
    return;
  }

  const char *response = sev->get_response();
  int len = sev->get_response_len();
  if (len <= 0 || response == nullptr) {
    response = "No data\n";
    len = strlen(response) + 1;
  }
  Server::send(sev->get_client(), response, len);
  if ('\0' != response[len - 1]) {
    // 这里强制性的给发送一个消息终结符，如果需要发送多条消息，需要调整
    char end = 0;
    Server::send(sev->get_client(), &end, 1);
  }

  // sev->done();
  VULCAN_LOG(trace, "Exit\n");
  return;
}

void SessionStage::handle_request(StageEvent *event) {
  SessionEvent *sev = dynamic_cast<SessionEvent *>(event);
  if (nullptr == sev) {
    VULCAN_LOG(error, "Cannot cat event to sessionEvent");
    return;
  }

  // TODO(Ziming Zhang): 将sql发送给parse_stage_，并且注册回调函数

  //   std::string sql = sev->get_request_buf();
  //   if (is_blank(sql.c_str())) {
  //     sev->done_immediate();
  //     return;
  //   }

  VULCAN_LOG(info, "SessionStage is handling event {}", sev->get_request_buf());
  sev->set_response("Hello, world!\n", strlen("Hello, world!\n") + 1);

  CompletionCallback *cb = new (std::nothrow) CompletionCallback(this, nullptr);
  if (cb == nullptr) {
    VULCAN_LOG(error, "Failed to new callback for SessionEvent");

    sev->done_immediate();
    return;
  }

  sev->push_callback(cb);

  //   SQLStageEvent *sql_event = new SQLStageEvent(sev, sql);
  //   parse_stage_->handle_event(sql_event);
  sev->done_immediate();
}

}  // namespace vulcan
