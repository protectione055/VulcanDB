// Copyright 2023 VulcanDB
# pragma once

#include <string.h>

namespace vulcan {
constexpr unsigned int ONE_KILO = 1024;
constexpr unsigned int ONE_MILLION = ONE_KILO * ONE_KILO;
constexpr unsigned int ONE_GIGA = ONE_MILLION * ONE_KILO;

#define MAX_CONNECTION_NUM "max_connection_num"

#define MAX_CONNECTION_NUM_DEFAULT ONE_KILO          // 默认最大连接数
#define PORT_DEFAULT 6688                            // 默认端口号
#define UNIX_SOCKET_PATH_DEFAULT "/tmp/vulcandb.sock"  // 默认unix socket路径
#define SOCKET_BUFFER_SIZE ONE_MILLION    // 默认socket缓冲区大小
#define MAX_MEM_BUFFER_SIZE 8 * ONE_KILO  // 默认内存缓冲区大小

#define VULCAN_HOME "VULCAN_HOME"
#define VULCAN_DATA_DIR "VULCAN_DATA_DIR"
#define VULCAN_LOG_DIR "VULCAN_LOG_DIR"
#define VULCAN_CONF_FILE "VULCAN_CONF_FILE"

#define SYS_OUTPUT_ERROR ",error:" << errno << ":" << strerror(errno)

#ifndef DEBUG_LOCK
#define MUTEXT_STATIC_INIT() PTHREAD_MUTEX_INITIALIZER
#define MUTEX_INIT(lock, attr) pthread_mutex_init(lock, attr)
#define MUTEX_DESTROY(lock) pthread_mutex_destroy(lock)
#define MUTEX_LOCK(lock) pthread_mutex_lock(lock)
#define MUTEX_UNLOCK(lock) pthread_mutex_unlock(lock)
#define MUTEX_TRYLOCK(lock) pthread_mutex_trylock(lock)

#define COND_INIT(cond, attr) pthread_cond_init(cond, attr)
#define COND_DESTROY(cond) pthread_cond_destroy(cond)
#define COND_WAIT(cond, mutex) pthread_cond_wait(cond, mutex)
#define COND_WAIT_TIMEOUT(cond, mutex, time, ret) \
  ret = pthread_cond_timedwait(cond, mutex, time)
#define COND_SIGNAL(cond) pthread_cond_signal(cond)
#define COND_BRAODCAST(cond) pthread_cond_broadcast(cond)
#endif  // DEBUG_LOCK

}  // namespace vulcan
