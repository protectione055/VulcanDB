// Copyright 2023 VulcanDB
#pragma once

#include <fcntl.h>
#include <unistd.h>

#include <filesystem>
#include <stdexcept>
#include <string>

namespace vulcan {

// 创建并返回一个临时目录
// @return 临时目录路径
std::filesystem::path create_temp_directory();

// 打开一个文件并对其上锁
// @param file_path 文件路径
// @param oflag 打开文件的方式
// @param args 打开文件的参数
// @return 文件描述符，-1表示文件因为被占用
// @throw std::runtime_error 打开文件失败
template <typename... Args>
int open_file_and_lock(const std::string& file_path, int oflag,
                       const Args&... args) {
  int fd = open(file_path.c_str(), oflag, args...);
  if (fd == -1) {
    if (errno == EACCES) {
      throw std::runtime_error("Permission Denied. Failed to open file " +
                               file_path);
    }
    throw std::runtime_error("Failed to open file " + file_path);
  }

  struct flock fl;
  switch (oflag) {
    case O_RDONLY:
      fl.l_type = F_RDLCK;
      break;
    case O_WRONLY:
      fl.l_type = F_WRLCK;
      break;
    case O_RDWR:
      fl.l_type = F_WRLCK;
      break;
    default:
      break;
  }
  fl.l_whence = SEEK_SET;  // SEEK_SET, SEEK_CUR, SEEK_END
  fl.l_start = 0;          // Offset from l_whence
  fl.l_len = 0;            // length, 0 = to EOF
  fl.l_pid = getpid();
  if (fcntl(fd, F_SETLK, &fl) == -1) {
    if (errno == EAGAIN) {
      close(fd);
      return -1;
    } else {
      throw std::runtime_error("Failed to lock file " + file_path);
    }
  }
  return fd;
}

// 将windows下的换行符替换为unix下的换行符
// @param file 转换的文件路径
void replace_win_newlines_to_unix(std::string fileresult_file,
                                  std::string new_file = "");

// 将字符串中的子串替换为新的子串
// @param resource_str 源字符串
// @param sub_str 要替换的子串
// @param new_str 新子串
// @return 替换后的字符串
std::string subreplace(std::string resource_str, std::string sub_str,
                       std::string new_str);

}  // namespace vulcan
