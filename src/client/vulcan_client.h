// Copyright 2023 VulcanDB
#pragma once

#include <sys/socket.h>

#include <string>

#include "common/defs.h"

namespace vulcan {

class VulcanClient {
 public:
  ~VulcanClient() = default;

 public:
  static VulcanClient *get_instance() {
    static VulcanClient instance;
    return &instance;
  }

  /**
   * @brief Initializes the VulcanClient object.
   *
   * This function processes the command line arguments, sets the server host,
   * server port, and Unix socket path, and establishes a connection to the
   * server.
   *
   * @param argc The number of command line arguments.
   * @param argv An array of command line argument strings.
   */
  void init(int argc, char *argv[]);

  /**
   * @brief Runs the client.
   *
   * This function reads a line of input from the console, sends it to the
   * server, and prints the server's response.
   */
  void run();

  void close();

  int get_sockfd() { return sockfd_; }

 private:
  VulcanClient() = default;
  VulcanClient(const VulcanClient &) = delete;

 private:
  /**
   * Reads a line of input from the console.
   *
   * @param prompt The prompt to display to the user.
   * @return A dynamically allocated buffer containing the read line, or nullptr
   * if an error occurred.
   */
  char *readline_from_cmd(const char *prompt);

  /**
   * @brief 接收并打印消息的函数
   *
   * 从给定的套接字接收消息(以\0结尾)并将其打印到标准输出。
   *
   * @param sockfd 套接字文件描述符
   * @param send_buf 发送缓冲区
   * @param buf_size 缓冲区大小
   * @return int 如果成功接收并打印消息，则返回0；否则返回-1
   */
  int recv_and_print_msg(int sockfd, char *send_buf,
                                       int buf_size);

  /**
   * Checks if the given command is an exit command.
   * An exit command can be "exit", "bye", or "\\q".
   *
   * @param cmd The command to check.
   * @return True if the command is an exit command, false otherwise.
   */
  bool is_exit_command(const char *cmd);

  int connect_tcp_sock(const char *server_host, int server_port);

  int connect_unix_sock(const char *unix_sock_path);

 private:
  const char *prompt_str_ = "vulcandb > ";
  char send_buf_[MAX_MEM_BUFFER_SIZE];  // 客户端读缓冲区
  std::string unix_socket_path_ = UNIX_SOCKET_PATH_DEFAULT;
  std::string server_host_ = "127.0.0.1";
  int server_port_ = std::stoi(PORT_DEFAULT);
  int sockfd_ = -1;
};

}  // namespace vulcan
