// Copyright 2023 VulcanDB
#include "backend/seda/kill_thread_stage.h"

#include <assert.h>

#include "backend/seda/stage_event.h"
#include "backend/seda/thread_pool.h"

namespace vulcan {

/**
 * Notify the pool and kill the thread
 * @param[in] event Pointer to event that must be handled.
 *
 * @post  Call never returns.  Thread is killed.  Pool is notified.
 */
void KillThreadStage::handle_event(StageEvent *event) {
  get_pool()->thread_kill();
  event->done();
  this->release_event();
  pthread_exit(0);
}

/**
 * Process properties of the classes
 * @pre class members are uninitialized
 * @post initializing the class members
 * @return the class object
 */
Stage *KillThreadStage::make_stage(const std::string &tag) {
  return new KillThreadStage(tag.c_str());
}

bool KillThreadStage::set_properties() {
  // nothing to do
  return true;
}

}  // namespace vulcan
