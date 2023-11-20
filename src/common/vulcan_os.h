// Copyright 2023 VulcanDB
#pragma once

#include <pthread.h>
#include <signal.h>
#include <stdio.h>

#include <iostream>

#include "common/defs.h"
#include "common/vulcan_utility.h"

namespace vulcan {

/**
 * Sets a signal handler for the specified signal.
 *
 * @param sig The signal number.
 * @param func The signal handler function.
 */
void setSignalHandler(int sig, sighandler_t func) {
  struct sigaction newsa, oldsa;
  sigemptyset(&newsa.sa_mask);
  newsa.sa_flags = 0;
  newsa.sa_handler = func;
  int rc = sigaction(sig, &newsa, &oldsa);
  if (rc) {
    std::cerr << "Failed to set signal " << sig << SYS_OUTPUT_FILE_POS
              << SYS_OUTPUT_ERROR << std::endl;
  }
}

/**
 * Sets the signal handler for the specified signals.
 *
 * @param func The signal handler function to set.
 */
void setSignalHandler(sighandler_t func) {
  setSignalHandler(SIGQUIT, func);
  setSignalHandler(SIGINT, func);
  setSignalHandler(SIGHUP, func);
  setSignalHandler(SIGTERM, func);
}

}  // namespace vulcan
