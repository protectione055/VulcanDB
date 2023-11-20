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
  char send_buf_[MAX_MEM_BUFFER_SIZE];
  std::string unix_socket_path_ = UNIX_SOCKET_PATH_DEFAULT;
  std::string server_host_ = "127.0.0.1";
  int server_port_ = PORT_DEFAULT;
  int sockfd_ = -1;
};

}  // namespace vulcan
