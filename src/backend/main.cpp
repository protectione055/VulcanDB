// Copyright 2023 VulcanDB
#include <getopt.h>
#include <signal.h>

#include <functional>
#include <iostream>

#include "backend/pidfile.h"
#include "backend/server.h"
#include "backend/vulcan_param.h"
#include "common/defs.h"
#include "common/os.h"
#include "common/string.h"
#include "common/vulcan_logger.h"
#include "common/vulcan_utility.h"

using namespace vulcan;

/*
 * Function declarations
 */

void init_parameter(int argc, char **argv);
void print_help();
void init_process(const vulcan::VulcanParam *config);
void cleanup_process(const vulcan::VulcanParam *config);
void *quit_thread_func(void *_signum);
void quit_signal_handle(int signum);
vulcan::Server *init_server(
    const vulcan::VulcanParam *config);
void prepare_init_seda();

/*
 * Global variables
 */

vulcan::Server *g_server = nullptr;

vulcan::VulcanParam *vulcan_param = vulcan::VulcanParam::get_instance();

/*
 * Any vulcandb server process begins execution here.
 */
int main(int argc, char **argv) {
  // TODO(Ziming Zhang): 从文件和环境变量中读取配置
  // 1. 从文件中读取配置
  // 2. 从环境变量中读取配置
  // 3. 从命令行参数中读取配置
  init_parameter(argc, argv);

  init_process(vulcan_param);

  VULCAN_LOG(info, "VulcanDB server listening on port {}",
             vulcan_param->get_server_port());
  g_server->serve();

  cleanup_process(vulcan_param);

  return 0;
}

// 解析命令行参数
void init_parameter(int argc, char **argv) {
  vulcan_param->default_init(argv[0]);

  // Process args
  int opt;
  int option_index = 0;
  extern char *optarg;
  struct option long_options[] = {
      {"config", required_argument, NULL, 'c'},
      {"port", optional_argument, NULL, 'p'},
      {"socket", optional_argument, NULL, 's'},
      {"data_dir", optional_argument, NULL, 'd'},
      {"log_dir", optional_argument, NULL, 'l'},
      {"help", no_argument, NULL, 'h'},
      {NULL, 0, NULL, 0},
  };
  while ((opt = getopt_long(argc, argv, "c:psdlh", long_options,
                            &option_index)) != -1) {
    switch (opt) {
      case 'c':
        vulcan_param->set_conf_file(optarg);
        break;
      case 'p':
        vulcan_param->set_server_port(atoi(optarg));
        break;
      case 's':
        vulcan_param->set_unix_socket_path(optarg);
        break;
      case 'h':
        print_help();
        break;
      default:
        std::cerr << "Unknown option" << std::endl;
        print_help();
        return;
    }
  }
}

void print_help() {
  std::cout << "Usage: vulcan_ctl [OPTION]..." << std::endl;
  std::cout << "  -c, --config=FILE        configuration file" << std::endl;
  std::cout << "  -p, --port=PORT          server port" << std::endl;
  std::cout << "  -s, --socket=PATH        unix socket path" << std::endl;
  std::cout << "  -d, --data_dir=PATH      data directory" << std::endl;
  std::cout << "  -l, --log_dir=PATH       log directory" << std::endl;
  std::cout << "  -h, --help               show this help" << std::endl;
  exit(0);
}

/**
 * @brief Function to handle quitting the thread.
 *
 * This function is called when a signal is received to quit the thread. It
 * takes the signal number as input and performs the necessary actions to
 * shutdown the server.
 *
 * @param _signum The signal number.
 * @return void* Returns a null pointer.
 */
void *quit_thread_func(void *_signum) {
  intptr_t signum = (intptr_t)_signum;
  VULCAN_LOG(info, "Receive signal: {}}", signum);
  if (g_server) {
    g_server->shutdown();
  }
  return nullptr;
}

/**
 * @brief Handles the quit signal.
 *
 * This function is called when a quit signal is received. It creates a new
 * thread to execute the quit_thread_func function with the given signum as an
 * argument.
 *
 * @param signum The signal number.
 */
void quit_signal_handle(int signum) {
  pthread_t tid;
  pthread_create(&tid, nullptr, quit_thread_func,
                 reinterpret_cast<void *>(signum));
}

/*
 * Initialize vulcan_ctl process
 */
void init_process(const vulcan::VulcanParam *config) {
  try {
    // create pid file
    vulcan::writePidFile(config->get_process_name().c_str());

     // Set Singal handling Fucntion
    setSignalHandler(quit_signal_handle);

    printf("Initialing vulcan_ctl process...\n");

    // Initialize runtime direcotries
    std::function<void(const char *, const std::filesystem::path &)>
        check_and_create_dir = [](const char *var_name,
                                  const std::filesystem::path &path) {
          std::printf("%s=%s\n", var_name, path.string().c_str());
          if (!std::filesystem::exists(path)) {
            std::filesystem::create_directory(path);
          }
        };
    check_and_create_dir(VULCAN_HOME, config->get_home());
    check_and_create_dir(VULCAN_DATA_DIR, config->get_data_dir());
    check_and_create_dir(VULCAN_LOG_DIR, config->get_log_dir());

    // Initialize vulcan_logger
    vulcan::VulcanLogger *vulcan_logger = vulcan::VulcanLogger::get_instance();
    vulcan_logger->init(config->get_log_dir(),
                        config->get_process_name() + std::to_string(getpid()),
                        config->get_log_level(),
                        config->get_console_log_level());

    // TODO(Ziming Zhang): 初始化seda

    // Initialize backend server
    g_server = init_server(config);
  } catch (const std::exception &e) {
    std::cerr << "init process failed: " << e.what() << std::endl;
    exit(1);
  }
}

vulcan::Server* init_server(const vulcan::VulcanParam *config) {
  int64_t listen_addr = INADDR_ANY;
  int64_t max_connection_num = MAX_CONNECTION_NUM_DEFAULT;
  int port = config->get_server_port();

  std::string conf_value = config->get_config(MAX_CONNECTION_NUM);
  if (!conf_value.empty()) {
    vulcan::str_to_val(conf_value, max_connection_num);
  }

  vulcan::ServerParam server_param;
  server_param.listen_addr = listen_addr;
  server_param.max_connection_num = max_connection_num;
  server_param.port = port;

  server_param.use_unix_socket = true;
  server_param.unix_socket_path = config->get_unix_socket_path();

  vulcan::Server *server = new vulcan::Server(server_param);
  return server;
}

void cleanup_process(const vulcan::VulcanParam *config) {
  VULCAN_LOG(info, "Cleaning up vulcan_ctl process...");
  // remove pid file
  vulcan::removePidFile();
  // remove unix socket file
  std::filesystem::remove(config->get_unix_socket_path());
}

// 注册stage的工厂函数
void prepare_init_seda() {
  //   static StageFactory session_stage_factory("SessionStage",
  //                                             &SessionStage::make_stage);
  //   static StageFactory resolve_stage_factory("ResolveStage",
  //                                             &ResolveStage::make_stage);
}