// Copyright 2023 VulcanDB
#include "common/os.h"

#include <signal.h>
#include <stdio.h>

#include <filesystem>
#include <iostream>
#include <string>

#include "common/defs.h"
#include "common/vulcan_logger.h"

namespace vulcan {

std::filesystem::path get_definite_path(const std::filesystem::path& path) {
  std::filesystem::path definite_path = path;
  if (definite_path.string().find("~/") == 0) {
    std::filesystem::path home_path = std::filesystem::path(getenv("HOME"));
    definite_path = home_path / definite_path.string().substr(2);
  }
  return definite_path;
}

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
