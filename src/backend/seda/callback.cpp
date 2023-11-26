// Copyright 2023 VulcanDB

#include "backend/seda/callback.h"

#include "backend/seda/stage.h"
#include "backend/seda/stage_event.h"
#include "common/vulcan_logger.h"

namespace vulcan {

extern bool &get_event_history_flag();

/**
 * @author Longda
 * @date   3/27/07
 *
 * Implementation of CompletionCallback class.
 */

// Constructor
CompletionCallback::CompletionCallback(Stage *trgt, CallbackContext *ctx)
    : target_stage_(trgt),
      context_(ctx),
      next_cb_(nullptr),
      ev_hist_flag_(get_event_history_flag()) {}

// Destructor
CompletionCallback::~CompletionCallback() {
  if (context_) {
    delete context_;
  }
  if (next_cb_) {
    delete next_cb_;
  }
}

// Push onto a callback stack
void CompletionCallback::push_callback(CompletionCallback *next) {
  ASSERT((!next_cb_), "%s", "cannot push a callback twice");

  next_cb_ = next;
}

// Pop off of a callback stack
CompletionCallback *CompletionCallback::pop_callback() {
  CompletionCallback *ret_val = next_cb_;

  next_cb_ = nullptr;
  return ret_val;
}

// One event is complete
void CompletionCallback::event_done(StageEvent *ev) {
  if (ev_hist_flag_) {
    ev->save_stage(target_stage_, StageEvent::CALLBACK_EV);
  }
  target_stage_->callback_event(ev, context_);
}

// Reschedule callback on target stage thread
void CompletionCallback::event_reschedule(StageEvent *ev) {
  target_stage_->add_event(ev);
}

void CompletionCallback::event_timeout(StageEvent *ev) {
  VULCAN_LOG(debug, "to call event_timeout for stage {}}",
             target_stage_->get_name());
  if (ev_hist_flag_) {
    ev->save_stage(target_stage_, StageEvent::TIMEOUT_EV);
  }
  target_stage_->timeout_event(ev, context_);
}

}  // namespace vulcan
