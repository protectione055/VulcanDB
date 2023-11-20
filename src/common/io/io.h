// Copyright 2023 VulcanDB
#pragma once

namespace vulcan {

/**
 * Writes data to a file descriptor.
 *
 * @param fd The file descriptor to write to.
 * @param buf A pointer to the data to be written.
 * @param size The size of the data to be written.
 * @return 0 if the write operation is successful, otherwise returns an error
 * code.
 */
int writen(int fd, const void *buf, int size);

/**
 * @brief Reads data from a file descriptor into a buffer.
 *
 * This function reads `size` bytes from the file descriptor `fd` into the
 * buffer `buf`. It continues reading until `size` bytes have been read or an
 * error occurs.
 *
 * @param fd The file descriptor to read from.
 * @param buf The buffer to store the read data.
 * @param size The number of bytes to read.
 * @return 0 if successful, otherwise an error code.
 */
int readn(int fd, void *buf, int size);

}  // namespace vulcan
