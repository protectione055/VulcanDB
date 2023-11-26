// Copyright 2023 VulcanDB

#include "backend/server.h"

#include <arpa/inet.h>
#include <errno.h>
#include <event2/event_compat.h>
#include <event2/thread.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "backend/seda/seda_config.h"
#include "backend/seda/seda_defs.h"
#include "backend/seda/session_event.h"
#include "backend/seda/session_stage.h"
#include "backend/session.h"
#include "common/defs.h"
#include "common/io/io.h"
#include "common/vulcan_logger.h"

namespace vulcan {

Stage *Server::session_stage_ = nullptr;

ServerParam::ServerParam() {
  listen_addr;
  max_connection_num;
  port;
}

Server::Server(ServerParam input_server_param)
    : server_param_(input_server_param) {
  started_ = false;
  server_socket_ = 0;
  event_base_ = nullptr;
  listen_ev_ = nullptr;
}

Server::~Server() {
  if (started_) {
    shutdown();
  }
}

void Server::init() {
  session_stage_ = SedaConfig::get_instance()->get_stage(SESSION_STAGE_NAME);
}

int Server::set_non_block(int fd) {
  int flags = fcntl(fd, F_GETFL);
  if (flags == -1) {
    VULCAN_LOG(info, "Failed to get flags of fd :{}. ", fd);
    return -1;
  }

  flags = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  if (flags == -1) {
    VULCAN_LOG(info, "Failed to set non-block flags of fd :{}. ", fd);
    return -1;
  }
  return 0;
}

void Server::close_connection(ConnectionContext *client_context) {
  VULCAN_LOG(info, "Close connection of {}.", client_context->addr);
  event_del(&client_context->read_event);
  ::close(client_context->fd);
  delete client_context->session;
  client_context->session = nullptr;
  delete client_context;
}

void Server::recv(int fd, int16_t ev, void *arg) {
  ConnectionContext *client = reinterpret_cast<ConnectionContext *>(arg);

  int data_len = 0;
  int read_len = 0;
  int buf_size = sizeof(client->buf);
  memset(client->buf, 0, buf_size);

  MUTEX_LOCK(&client->mutex);
  // 持续接收消息，直到遇到'\0'。将'\0'遇到的后续数据直接丢弃没有处理，因为目前仅支持一收一发的模式
  while (true) {
    read_len = ::read(client->fd, client->buf + data_len, buf_size - data_len);
    if (read_len < 0) {
      if (errno == EAGAIN) {
        continue;
      }
      break;
    }
    if (read_len == 0) {
      break;
    }

    if (read_len + data_len > buf_size) {
      data_len += read_len;
      break;
    }

    bool msg_end = false;
    for (int i = 0; i < read_len; i++) {
      if (client->buf[data_len + i] == 0) {
        data_len += i + 1;
        msg_end = true;
        break;
      }
    }

    if (msg_end) {
      break;
    }

    data_len += read_len;
  }

  MUTEX_UNLOCK(&client->mutex);

  if (data_len > buf_size) {
    VULCAN_LOG(warn, "The length of sql exceeds the limitation {}\n", buf_size);
    close_connection(client);
    return;
  }
  if (read_len == 0) {
    VULCAN_LOG(info, "The peer has been closed {}\n", client->addr);
    close_connection(client);
    return;
  } else if (read_len < 0) {
    VULCAN_LOG(error, "Failed to read socket of {}, {}\n", client->addr,
               strerror(errno));
    close_connection(client);
    return;
  }

  VULCAN_LOG(info, "receive command(size={}): {}", data_len, client->buf);
  SessionEvent *sev = new SessionEvent(client);
  session_stage_->add_event(sev);
  //   const char *ack = "ack";
  //   send(client, ack, strlen(ack) + 1);
  VULCAN_LOG(info, "Server::recv 执行成功");
}

// 这个函数仅负责发送数据，至于是否是一个完整的消息，由调用者控制
int Server::send(ConnectionContext *client, const char *buf, int data_len) {
  if (buf == nullptr || data_len == 0) {
    return 0;
  }

  MUTEX_LOCK(&client->mutex);
  int ret = writen(client->fd, buf, data_len);
  if (ret < 0) {
    VULCAN_LOG(error, "Failed to send data back to client. ret={}, error={}",
               ret, strerror(errno));
    MUTEX_UNLOCK(&client->mutex);

    close_connection(client);
    return -1;
  }

  MUTEX_UNLOCK(&client->mutex);
  return 0;
}

void Server::accept(int fd, int16_t ev, void *arg) {
  Server *instance = reinterpret_cast<Server *>(arg);
  struct sockaddr_in addr;
  socklen_t addrlen = sizeof(addr);

  int ret = 0;

  int client_fd = ::accept(fd, (struct sockaddr *)&addr, &addrlen);
  if (client_fd < 0) {
    VULCAN_LOG(error, "Failed to accept client's connection, {}",
               strerror(errno));
    return;
  }

  char ip_addr[24];
  if (inet_ntop(AF_INET, &addr.sin_addr, ip_addr, sizeof(ip_addr)) == nullptr) {
    VULCAN_LOG(error, "Failed to get ip address of client, {}",
               strerror(errno));
    ::close(client_fd);
    return;
  }
  std::stringstream address;
  address << ip_addr << ":" << addr.sin_port;
  std::string addr_str = address.str();

  ret = instance->set_non_block(client_fd);
  if (ret < 0) {
    VULCAN_LOG(error, "Failed to set socket of {} as non blocking, {}",
               addr_str.c_str(), strerror(errno));
    ::close(client_fd);
    return;
  }

  if (!instance->server_param_.use_unix_socket) {
    // unix socket不支持设置NODELAY
    int yes = 1;
    ret = setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &yes, sizeof(yes));
    if (ret < 0) {
      VULCAN_LOG(error,
                 "Failed to set socket of {} option as : TCP_NODELAY {}\n",
                 addr_str.c_str(), strerror(errno));
      ::close(client_fd);
      return;
    }
  }

  ConnectionContext *client_context = new ConnectionContext();
  memset(client_context, 0, sizeof(ConnectionContext));
  client_context->fd = client_fd;
  snprintf(client_context->addr, sizeof(client_context->addr), "%s",
           addr_str.c_str());
  pthread_mutex_init(&client_context->mutex, nullptr);

  event_set(&client_context->read_event, client_context->fd,
            EV_READ | EV_PERSIST, recv, client_context);

  ret = event_base_set(instance->event_base_, &client_context->read_event);
  if (ret < 0) {
    VULCAN_LOG(
        error,
        "Failed to do event_base_set for read event of {} into libevent, {}",
        client_context->addr, strerror(errno));
    delete client_context;
    ::close(instance->server_socket_);
    return;
  }

  ret = event_add(&client_context->read_event, nullptr);
  if (ret < 0) {
    VULCAN_LOG(error,
               "Failed to event_add for read event of {} into libevent, {}",
               client_context->addr, strerror(errno));
    delete client_context;
    ::close(instance->server_socket_);
    return;
  }

  client_context->session = new Session(Session::default_session());
  VULCAN_LOG(info, "Accepted connection from {}\n", client_context->addr);
}

int Server::start() {
  if (server_param_.use_unix_socket) {
    return start_unix_socket_server();
  } else {
    return start_tcp_server();
  }
}
int Server::start_tcp_server() {
  int ret = 0;
  struct sockaddr_in sa;

  server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket_ < 0) {
    VULCAN_LOG(error, "socket(): can not create server socket: {}.",
               strerror(errno));
    return -1;
  }

  int yes = 1;
  ret = setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
  if (ret < 0) {
    VULCAN_LOG(error, "Failed to set socket option of reuse address: {}.",
               strerror(errno));
    ::close(server_socket_);
    return -1;
  }

  ret = set_non_block(server_socket_);
  if (ret < 0) {
    VULCAN_LOG(error, "Failed to set socket option non-blocking:{}. ",
               strerror(errno));
    ::close(server_socket_);
    return -1;
  }

  memset(&sa, 0, sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_port = htons(server_param_.port);
  sa.sin_addr.s_addr = htonl(server_param_.listen_addr);

  ret = bind(server_socket_, (struct sockaddr *)&sa, sizeof(sa));
  if (ret < 0) {
    VULCAN_LOG(error, "bind(): can not bind server socket, {}",
               strerror(errno));
    ::close(server_socket_);
    return -1;
  }

  ret = listen(server_socket_, server_param_.max_connection_num);
  if (ret < 0) {
    VULCAN_LOG(error, "listen(): can not listen server socket, {}",
               strerror(errno));
    ::close(server_socket_);
    return -1;
  }
  VULCAN_LOG(info, "Listen on port {}", server_param_.port);

  listen_ev_ = event_new(event_base_, server_socket_, EV_READ | EV_PERSIST,
                         accept, this);
  if (listen_ev_ == nullptr) {
    VULCAN_LOG(error, "Failed to create listen event, {}.", strerror(errno));
    ::close(server_socket_);
    return -1;
  }

  ret = event_add(listen_ev_, nullptr);
  if (ret < 0) {
    VULCAN_LOG(error, "event_add(): can not add accept event into libevent, {}",
               strerror(errno));
    ::close(server_socket_);
    return -1;
  }

  started_ = true;
  VULCAN_LOG(info, "VulcanDB server start success");
  return 0;
}

int Server::start_unix_socket_server() {
  int ret = 0;
  server_socket_ = socket(PF_UNIX, SOCK_STREAM, 0);
  if (server_socket_ < 0) {
    VULCAN_LOG(error, "socket(): can not create unix socket: {}.",
               strerror(errno));
    return -1;
  }

  ret = set_non_block(server_socket_);
  if (ret < 0) {
    VULCAN_LOG(error, "Failed to set socket option non-blocking:{}. ",
               strerror(errno));
    ::close(server_socket_);
    return -1;
  }

  unlink(server_param_.unix_socket_path.c_str());

  struct sockaddr_un sockaddr;
  memset(&sockaddr, 0, sizeof(sockaddr));
  sockaddr.sun_family = PF_UNIX;
  snprintf(sockaddr.sun_path, sizeof(sockaddr.sun_path), "%s",
           server_param_.unix_socket_path.c_str());

  ret = bind(server_socket_, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
  if (ret < 0) {
    VULCAN_LOG(error, "bind(): can not bind server socket(path={}), {}",
               sockaddr.sun_path, strerror(errno));
    ::close(server_socket_);
    return -1;
  }

  ret = listen(server_socket_, server_param_.max_connection_num);
  if (ret < 0) {
    VULCAN_LOG(error, "listen(): can not listen server socket, {}",
               strerror(errno));
    ::close(server_socket_);
    return -1;
  }
  VULCAN_LOG(info, "Listen on unix socket: {}", sockaddr.sun_path);

  listen_ev_ = event_new(event_base_, server_socket_, EV_READ | EV_PERSIST,
                         accept, this);
  if (listen_ev_ == nullptr) {
    VULCAN_LOG(error, "Failed to create listen event, {}.", strerror(errno));
    ::close(server_socket_);
    return -1;
  }

  ret = event_add(listen_ev_, nullptr);
  if (ret < 0) {
    VULCAN_LOG(error, "event_add(): can not add accept event into libevent, {}",
               strerror(errno));
    ::close(server_socket_);
    return -1;
  }

  started_ = true;
  VULCAN_LOG(info, "VulcanDB server start success");
  return 0;
}

int Server::serve() {
  evthread_use_pthreads();
  event_base_ = event_base_new();
  if (event_base_ == nullptr) {
    VULCAN_LOG(error, "Failed to create event base, {}.", strerror(errno));
    exit(-1);
  }

  int retval = start();
  if (retval == -1) {
    VULCAN_LOG(panic, "Failed to start network");
    exit(-1);
  }

  event_base_dispatch(event_base_);

  if (listen_ev_ != nullptr) {
    event_del(listen_ev_);
    event_free(listen_ev_);
    listen_ev_ = nullptr;
  }

  if (event_base_ != nullptr) {
    event_base_free(event_base_);
    event_base_ = nullptr;
  }

  started_ = false;
  VULCAN_LOG(info, "Server quit");
  return 0;
}

void Server::shutdown() {
  VULCAN_LOG(info, "Server shutting down");

  // cleanup
  if (event_base_ != nullptr && started_) {
    started_ = false;
    event_base_loopexit(event_base_, nullptr);
  }
}

}  // namespace vulcan
