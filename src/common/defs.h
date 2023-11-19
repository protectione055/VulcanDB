// Copyright 2023 VulcanDB
# pragma once

#include <string.h>

namespace vulcan {

#define VULCAN_HOME "VULCAN_HOME"
#define VULCAN_DATA_DIR "VULCAN_DATA_DIR"
#define VULCAN_LOG_DIR "VULCAN_LOG_DIR"
#define VULCAN_CONF_FILE "VULCAN_CONF_FILE"

constexpr unsigned int ONE_KILO = 1024;
constexpr unsigned int ONE_MILLION = ONE_KILO * ONE_KILO;
constexpr unsigned int ONE_GIGA = ONE_MILLION * ONE_KILO;

#define SYS_OUTPUT_ERROR ",error:" << errno << ":" << strerror(errno)

}  // namespace vulcan
