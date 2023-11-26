// Copyright 2023 VulcanDB
#pragma once

#include <signal.h>

#include <filesystem>

namespace vulcan {

std::filesystem::path get_definite_path(const std::filesystem::path& path);

void setSignalHandler(int sig, sighandler_t func);

void setSignalHandler(sighandler_t func);

}  // namespace vulcan
