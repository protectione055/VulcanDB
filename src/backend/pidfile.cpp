// Copyright 2023 VulcanDB

#include "backend/pidfile.h"

#include <libgen.h>
#include <paths.h>
#include <sys/param.h>
#include <unistd.h>

#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

#include "common/defs.h"

namespace vulcan {

std::string &getPidPath() {
  static std::string path;

  return path;
}

void setPidPath(const char *progName) {
  std::string &path = getPidPath();

  if (progName != NULL) {
    path = std::string(_PATH_TMP) + progName + ".pid";
  } else {
    path = "";
  }
}

int writePidFile(const char *progName) {
  assert(progName);
  std::ofstream ostr;
  int rv = 1;

  setPidPath(progName);
  std::string path = getPidPath();
  ostr.open(path.c_str(), std::ios::trunc);
  if (ostr.good()) {
    ostr << getpid() << std::endl;
    ostr.close();
    rv = 0;
  } else {
    rv = errno;
    std::cerr << "error opening PID file " << path.c_str() << SYS_OUTPUT_ERROR
              << std::endl;
  }

  return rv;
}

void removePidFile(void) {
  std::string path = getPidPath();
  if (!path.empty()) {
    unlink(path.c_str());
    setPidPath(NULL);
  }
  return;
}

}  // namespace vulcan
