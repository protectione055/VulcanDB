// Copyright 2023 VulcanDB
#pragma once

#include <string>

#include "backend/seda/stage.h"

namespace vulcan {

/**
 * seda::stage使用说明：
 * 这里利用seda的线程池与调度。stage是一个事件处理的几个阶段。
 * 目前包括session,parse,execution和storage
 * 每个stage使用handleEvent函数处理任务，并且使用StageEvent::pushCallback注册回调函数。
 * 这时当调用StageEvent::done(Immediate)时，就会调用该事件注册的回调函数。
 */
class SessionStage : public Stage {
 public:
  ~SessionStage();
  static Stage *make_stage(const std::string &tag);

 protected:
  // common function
  explicit SessionStage(const char *tag);
  bool set_properties() override;

  bool initialize() override;
  void cleanup() override;
  void handle_event(StageEvent *event) override;
  void callback_event(StageEvent *event,
                      CallbackContext *context) override;

 protected:
  void handle_input(StageEvent *event);

  void handle_request(StageEvent *event);

 private:
  Stage *plan_cache_stage_ = nullptr;
};

}  // namespace vulcan
