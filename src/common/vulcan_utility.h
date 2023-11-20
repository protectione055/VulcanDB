// Copyright 2023 VulcanDB
#pragma once

#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>

#include <filesystem>
#include <stdexcept>
#include <string>

namespace vulcan {

#define SYS_OUTPUT_FILE_POS \
  ", File:" << __FILE__ << ", line:" << __LINE__ << ",function:" << __FUNCTION__
#define SYS_OUTPUT_ERROR ",error:" << errno << ":" << strerror(errno)

/**
 * Converts a string to a value of type T.
 *
 * @param str The string to convert.
 * @param val The converted value will be stored in this variable.
 * @param radix The radix to use for conversion (default is decimal).
 * @return True if the conversion was successful, false otherwise.
 */
template <class T>
bool str_to_val(const std::string &str, T &val,
                std::ios_base &(*radix)(std::ios_base &) = std::dec) {
  bool success = true;
  std::istringstream is(str);
  if (!(is >> radix >> val)) {
    val = 0;
    success = false;
  }
  return success;
}

/**
 * @brief Checks if a string is blank.
 *
 * A string is considered blank if it is nullptr or contains only whitespace
 * characters.
 *
 * @param s The string to check.
 * @return True if the string is blank, false otherwise.
 */
bool is_blank(const char *s);

/**
 * @brief Replaces all occurrences of a substring in a string with a new
 * substring.
 *
 * This function takes a source string, a substring to be replaced, and a new
 * substring. It replaces all occurrences of the substring in the source string
 * with the new substring. The modified string is then returned.
 *
 * @param resource_str The source string in which the replacement will be
 * performed.
 * @param sub_str The substring to be replaced.
 * @param new_str The new substring that will replace the occurrences of the old
 * substring.
 * @return The modified string after the replacement.
 */
std::string subreplace(std::string resource_str, std::string sub_str,
                       std::string new_str);

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
