// Copyright 2023 VulcanDB
#include <getopt.h>

#include <functional>
#include <iostream>

#include "common/defs.h"
#include "common/vulcan_logger.h"
#include "common/vulcan_param.h"

// global variables
vulcan::VulcanLogger *vulcan_logger = vulcan::VulcanLogger::get_instance();
vulcan::VulcanParam *vulcan_param = vulcan::VulcanParam::get_instance();

// function declarations
void parse_parameter(int argc, char **argv);
void usage();
void init_process(const vulcan::VulcanParam *config);

/*
 * Any vulcandb server process begins execution here.
 */
int main(int argc, char **argv) {
  parse_parameter(argc, argv);

  init_process(vulcan_param);

  return 0;
}

// 解析命令行参数
// 优先级：命令行参数 > (环境变量) > 配置文件
void parse_parameter(int argc, char **argv) {
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
        usage();
        break;
      default:
        std::cerr << "Unknown option" << std::endl;
        usage();
        return;
    }
  }
}

void usage() {
  std::cout << "Usage: vulcan_ctl [OPTION]..." << std::endl;
  std::cout << "  -c, --config=FILE        configuration file" << std::endl;
  std::cout << "  -p, --port=PORT          server port" << std::endl;
  std::cout << "  -s, --socket=PATH        unix socket path" << std::endl;
  std::cout << "  -d, --data_dir=PATH      data directory" << std::endl;
  std::cout << "  -l, --log_dir=PATH       log directory" << std::endl;
  std::cout << "  -h, --help               show this help" << std::endl;
  exit(0);
}

// inittialize vulcan_ctl process
void init_process(const vulcan::VulcanParam *config) {
  try {
    // TODO: 读取配置文件
    // TODO: 读取环境变量

    // Initialize runtime direcotries
    std::function<void(const char *, const std::filesystem::path &)>
        check_and_create_dir =
            [](const char *var_name, const std::filesystem::path &path) {
              vulcan_logger->info("{}={}", var_name, path.string());
              if (!std::filesystem::exists(path)) {
                vulcan_logger->info("creating directory: {}", path.string());
                std::filesystem::create_directory(path);
              }
            };
    check_and_create_dir(VULCAN_HOME, config->get_home());
    check_and_create_dir(VULCAN_DATA_DIR, config->get_data_dir());
    check_and_create_dir(VULCAN_LOG_DIR, config->get_log_dir());

    // Initialize vulcan_logger
    vulcan_logger->init(vulcan_param->get_log_dir(),
                        vulcan_param->get_process_name());
  } catch (const std::exception &e) {
    vulcan_logger->error("VulcanDB init failed. {}", e.what());
    exit(1);
  }
}
