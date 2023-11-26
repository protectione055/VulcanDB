// Copyright 2023 VulcanDB

#include "client/vulcan_client.h"

#include <alloca.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <iostream>

#include "common/io/io.h"
#include "common/string.h"
#include "common/vulcan_utility.h"

namespace vulcan {

void VulcanClient::init(int argc, char *argv[]) {
  try {
    // Process args
    int opt;
    extern char *optarg;
    while ((opt = getopt(argc, argv, "s:h:p:")) > 0) {
      switch (opt) {
        case 's':
          unix_socket_path_ = ::optarg;
          break;
        case 'p':
          server_port_ = atoi(::optarg);
          break;
        case 'h':
          server_host_ = ::optarg;
          break;
      }
    }

    // Try to connect to server
    if (server_host_ == "" || server_host_ == "localhost" ||
        server_host_ == "127.0.0.1") {
      sockfd_ = connect_unix_sock(unix_socket_path_.c_str());
    } else {
      sockfd_ = connect_tcp_sock(server_host_.c_str(), server_port_);
    }
    if (sockfd_ < 0) {
      std::cerr << "Failed to connect to server" << std::endl;
      exit(1);
    }

  } catch (const std::exception &e) {
    std::cerr << "Failed to init client param: " << e.what() << std::endl;
    exit(1);
  }
}

void VulcanClient::run() {
  while (true) {
    // 从标准输入读取用户输入的查询语句
    char *input_command = readline_from_cmd(prompt_str_);
    if (input_command == nullptr || is_exit_command(input_command)) {
      break;
    }

    if (is_blank(input_command)) {
      free(input_command);
      continue;
    }

    // 向数据库服务端发送输入的查询语句
    if (writen(sockfd_, input_command, strlen(input_command) + 1) != 0) {
      std::cerr << "send error: " << strerror(errno) << std::endl;
      free(input_command);
      exit(1);
    }
    free(input_command);

    // 从数据库服务端接收并打印查询结果
    int ret = recv_and_print_msg(sockfd_, send_buf_, MAX_MEM_BUFFER_SIZE);
    if (ret < 0) {
      fprintf(stderr, "Connection was broken: %s\n", strerror(errno));
      break;
    }
    if (ret == 1) {
      printf("Connection has been closed\n");
      break;
    }
  }
}

int VulcanClient::recv_and_print_msg(int sockfd, char *send_buf, int buf_size) {
  int len = 0;
  memset(send_buf, 0, buf_size);
  while ((len = recv(sockfd, send_buf, buf_size, 0)) > 0) {
    bool msg_end = false;
    for (int i = 0; i < len; i++) {
      if ('\0' == send_buf[i]) {
        msg_end = true;
        break;
      }
      std::cout << send_buf[i];
    }
    if (msg_end) {
      break;
    }
    memset(send_buf, 0, buf_size);
  }
  std::cout << std::endl;
  return len < 0 ? -1 : 0;
}

void VulcanClient::close() {
  if (sockfd_ > 0) {
    ::close(sockfd_);
  }
}

char *VulcanClient::readline_from_cmd(const char *prompt) {
  char *buffer = new char[MAX_MEM_BUFFER_SIZE];
  if (nullptr == buffer) {
    fprintf(stderr, "failed to alloc line buffer");
    return nullptr;
  }
  fprintf(stdout, "%s", prompt);
  char *s = fgets(buffer, MAX_MEM_BUFFER_SIZE, stdin);
  if (nullptr == s) {
    fprintf(stderr, "failed to read message from console");
    free(buffer);
    return nullptr;
  }
  return buffer;
}

bool VulcanClient::is_exit_command(const char *cmd) {
  return 0 == strncasecmp("exit", cmd, 4) || 0 == strncasecmp("bye", cmd, 3) ||
         0 == strncasecmp("\\q", cmd, 2);
}

int VulcanClient::connect_unix_sock(const char *unix_sock_path) {
  int sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
  if (sockfd < 0) {
    fprintf(stderr, "failed to create unix socket. %s", strerror(errno));
    return -1;
  }

  struct sockaddr_un sockaddr;
  memset(&sockaddr, 0, sizeof(sockaddr));
  sockaddr.sun_family = PF_UNIX;
  snprintf(sockaddr.sun_path, sizeof(sockaddr.sun_path), "%s", unix_sock_path);

  if (connect(sockfd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) < 0) {
    fprintf(stderr,
            "failed to connect to server. unix socket path '%s'. error %s",
            sockaddr.sun_path, strerror(errno));
    ::close(sockfd);
    return -1;
  }
  return sockfd;
}

int VulcanClient::connect_tcp_sock(const char *server_host, int server_port) {
  struct hostent *host;
  struct sockaddr_in serv_addr;

  if ((host = gethostbyname(server_host)) == NULL) {
    fprintf(stderr, "gethostbyname failed. errmsg=%d:%s\n", errno,
            strerror(errno));
    return -1;
  }

  int sockfd;
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    fprintf(stderr, "create socket error. errmsg=%d:%s\n", errno,
            strerror(errno));
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(server_port);
  serv_addr.sin_addr = *((struct in_addr *)host->h_addr);
  bzero(&(serv_addr.sin_zero), 8);

  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) ==
      -1) {
    fprintf(stderr, "Failed to connect. errmsg=%d:%s\n", errno,
            strerror(errno));
    ::close(sockfd);
    return -1;
  }
  return sockfd;
}

}  // namespace vulcan
