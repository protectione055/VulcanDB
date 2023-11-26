// Copyright 2023 VulcanDB
#include "backend/seda/timeout_info.h"

#include <sys/time.h>

#include "common/mutex.h"

namespace vulcan {

TimeoutInfo::TimeoutInfo(time_t deadLine)
    : deadline_(deadLine), is_timed_out_(false), ref_cnt_(0) {
  MUTEX_INIT(&mutex_, NULL);
}

TimeoutInfo::~TimeoutInfo() {
  // unlock mutex_ as we locked it before 'delete this'
  MUTEX_UNLOCK(&mutex_);

  MUTEX_DESTROY(&mutex_);
}

void TimeoutInfo::attach() {
  MUTEX_LOCK(&mutex_);
  ref_cnt_++;
  MUTEX_UNLOCK(&mutex_);
}

void TimeoutInfo::detach() {
  MUTEX_LOCK(&mutex_);
  if (0 == --ref_cnt_) {
    delete this;
    return;
  }
  MUTEX_UNLOCK(&mutex_);
}

bool TimeoutInfo::has_timed_out() {
  MUTEX_LOCK(&mutex_);
  bool ret = is_timed_out_;
  if (!is_timed_out_) {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    ret = is_timed_out_ = (tv.tv_sec >= deadline_);
  }
  MUTEX_UNLOCK(&mutex_);

  return ret;
}

}  // namespace vulcan
