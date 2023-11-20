// Copyright 2023 VulcanDB

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <termios.h>
#include <unistd.h>

#include "client/vulcan_client.h"

vulcan::VulcanClient *client = vulcan::VulcanClient::get_instance();

int main(int argc, char *argv[]) {
  client->init(argc, argv);
  client->run();
  client->close();

  return 0;
}
