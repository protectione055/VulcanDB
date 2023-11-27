// Copyright 2023 VulcanDB
#include "backend/vulcan_param.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>
#include <string>

#include "common/defs.h"
#include "common/vulcan_logger.h"

namespace vulcan {

// Test fixture for VulcanParam class
class VulcanParamTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Set up any necessary resources before each test
    vulcan_param_ = VulcanParam::get_instance();
    vulcan_param_->set_conf_file(
        "/home/zzm/projects/VulcanDB/tests/backend/vulcan_param_test.ini");
    vulcan_param_->load_conf_file();
  }

  void TearDown() override {
    // Clean up any resources after each test
  }

  VulcanParam* vulcan_param_;
};

// Test case for VulcanParam::init()
TEST_F(VulcanParamTest, DefaultInitTest) {
  // Call the method under test
  ASSERT_EQ(vulcan_param_->get(VULCAN_HOME), DEFAULT_HOME);
  ASSERT_EQ(vulcan_param_->get(VULCAN_LOG_LEVEL), DEFAULT_LOG_LEVEL);
  ASSERT_EQ(vulcan_param_->get(MAX_CONNECTION_NUM), MAX_CONNECTION_NUM_DEFAULT);
  // Perform assertions
  // ...
}

// Test case for VulcanParam::set() / get()
TEST_F(VulcanParamTest, SetTest) {
  // Set up test data
  std::string key = "VULCAN_HOME";
  std::string value = "/path/to/vulcan/home";

  // Call the method under test
  vulcan_param_->set(key, value);

  // Perform assertions
  std::string retrieved_value = vulcan_param_->get(key);
  ASSERT_EQ(retrieved_value, value);
}

// Test case for VulcanParam::get_max_connection_num()
TEST_F(VulcanParamTest, GetMaxConnectionNumTest) {
  // Set up test data
  std::string key = MAX_CONNECTION_NUM;
  std::string value = "100";
  vulcan_param_->set(key, value);

  // Call the method under test
  int max_connection_num = vulcan_param_->get_max_connection_num();

  // Perform assertions
  ASSERT_EQ(max_connection_num, 100);
}

// Test case for VulcanParam::get_log_level()
TEST_F(VulcanParamTest, GetLogLevelTest) {
  // Set up test data
  std::string key = "VULCAN_LOG_LEVEL";
  std::string value = "3";
  vulcan_param_->set(key, value);

  // Call the method under test
  LOG_LEVEL log_level = vulcan_param_->get_log_level();

  // Perform assertions
  ASSERT_EQ(log_level, LOG_LEVEL::INFO);
}

// Test case for VulcanParam::get_console_log_level()
TEST_F(VulcanParamTest, GetConsoleLogLevelTest) {
  // Set up test data
  std::string key = "VULCAN_CONSOLE_LOG_LEVEL";
  std::string value = "3";
  vulcan_param_->set(key, value);

  // Call the method under test
  LOG_LEVEL console_log_level = vulcan_param_->get_console_log_level();

  // Perform assertions
  ASSERT_EQ(console_log_level, LOG_LEVEL::INFO);
}

// Test case for VulcanParam::get_server_port()
TEST_F(VulcanParamTest, GetServerPortTest) {
  // Set up test data
  std::string key = "VULCAN_PORT";
  std::string value = "8080";
  vulcan_param_->set(key, value);

  // Call the method under test
  int server_port = vulcan_param_->get_server_port();

  // Perform assertions
  ASSERT_EQ(server_port, 8080);
}

}  // namespace vulcan
