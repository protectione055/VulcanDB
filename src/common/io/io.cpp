// Copyright 2023 VulcanDB

#include "common/io/io.h"

#include <dirent.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#include "common/io/resp_protocol.h"

namespace vulcan {

/**
 * Writes data to a file descriptor.
 *
 * @param fd The file descriptor to write to.
 * @param buf A pointer to the buffer containing the data to be written.
 * @param size The number of bytes to write.
 * @return 0 on success, or an error code on failure.
 */
int writen(int fd, const void *buf, int size) {
  const char *tmp = (const char *)buf;
  while (size > 0) {
    const ssize_t ret = ::write(fd, tmp, size);
    if (ret >= 0) {
      tmp += ret;
      size -= ret;
      continue;
    }
    const int err = errno;
    if (EAGAIN != err && EINTR != err) return err;
  }
  return 0;
}

/**
 * @brief Reads data from a file descriptor into a buffer.
 *
 * @param fd The file descriptor to read from.
 * @param buf The buffer to store the read data.
 * @param size The number of bytes to read.
 * @return Returns 0 on success, -1 if the end of file is reached, or an error
 * code if an error occurs.
 */
int readn(int fd, void *buf, int size) {
  char *tmp = reinterpret_cast<char *>(buf);
  while (size > 0) {
    const ssize_t ret = ::read(fd, tmp, size);
    if (ret > 0) {
      tmp += ret;
      size -= ret;
      continue;
    }
    if (0 == ret) return -1;  // end of file

    const int err = errno;
    if (EAGAIN != err && EINTR != err) return err;
  }
  return 0;
}

}  // namespace vulcan
