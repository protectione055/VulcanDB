// Copyright 2023 VulcanDB
#pragma once

#include <string>

#include "common/defs.h"
#include "libevent/include/event.h"

namespace vulcan {

class Session;
class SessionStage;
class SedaConfig;
class Stage;

typedef struct _ConnectionContext {
  Session *session;
  int fd;
  struct event read_event;
  pthread_mutex_t mutex;
  char addr[24];
  char buf[SOCKET_BUFFER_SIZE];
} ConnectionContext;

class ServerParam {
 public:
  ServerParam();

  ServerParam(const ServerParam &other) = default;
  ~ServerParam() = default;

 public:
  // accpet client's address, default is INADDR_ANY, means accept every address
  int64_t listen_addr;

  int max_connection_num;
  // server listing port
  int port;

  std::string unix_socket_path;

  // 如果使用标准输入输出作为通信条件，就不再监听端口
  bool use_unix_socket = false;
};

class Server {
 public:
  explicit Server(ServerParam input_server_param);
  ~Server();

 public:
  static void init();
  static int send(ConnectionContext *client, const char *buf, int data_len);

 public:
  int serve();
  void shutdown();

 private:
  static void accept(int fd, int16_t ev, void *arg);
  // close connection
  static void close_connection(ConnectionContext *client_context);
  static void recv(int fd, int16_t ev, void *arg);

 private:
  int set_non_block(int fd);
  int start();
  int start_tcp_server();
  int start_unix_socket_server();

 private:
  bool started_;

  int server_socket_;
  struct event_base *event_base_;
  struct event *listen_ev_;
  ServerParam server_param_;

  static Stage *session_stage_;
};

class Communicator {
 public:
  virtual ~Communicator() = default;
  virtual int init(const ServerParam &server_param) = 0;
  virtual int start() = 0;
  virtual int stop() = 0;
};

}  // namespace vulcan
