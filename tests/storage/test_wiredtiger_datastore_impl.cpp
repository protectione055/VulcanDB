// Copyright 2023 VulcanDB
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>
#include <map>
#include <memory>

#include "storage/datastore/wiredtiger_datastore_impl.h"

using namespace vulcan;

class WiredTigerDataStoreTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Set up the test environment
    WiredTigerDataStoreConfig config("/tmp/vulcan/", "test");
    datastore_ = std::make_unique<WiredTigerDataStore>();
    EXPECT_EQ(datastore_->open_datastore_instance(&config), 0);
  }

  void TearDown() override {
    // Clean up the test environment
    datastore_->close_datastore_instance();
  }

 protected:
  std::unique_ptr<DataStoreInterface> datastore_;
};

TEST_F(WiredTigerDataStoreTest, GetTest) {
  // Arrange
  auto session_ = datastore_->new_datastore_session();
  EXPECT_NE(session_, nullptr);
  const char* key1 = "key1";
  const char* value1 = "value1";
  session_->upsert_kv(key1, value1);

  // Act
  const char* result1 = session_->get(key1);

  // Assert
  EXPECT_STREQ(result1, value1);
}

TEST_F(WiredTigerDataStoreTest, InsertTest) {
  // Arrange
  auto session_ = datastore_->new_datastore_session();
  EXPECT_NE(session_, nullptr);
  const char* key1 = "key1";
  const char* value1 = "value1";
  session_->delete_kv(key1);

  // Act
  bool inserted = session_->insert_kv(key1, value1);

  // Assert
  EXPECT_TRUE(inserted);
  const char* result1 = session_->get(key1);
  EXPECT_STREQ(result1, value1);
}

TEST_F(WiredTigerDataStoreTest, UpsertTest) {
  // Arrange
  auto session_ = datastore_->new_datastore_session();
  EXPECT_NE(session_, nullptr);
  const char* key1 = "key1";
  const char* value1 = "value1";

  // Act
  session_->upsert_kv(key1, value1);
  const char* result1 = session_->get(key1);

  // Assert
  EXPECT_STREQ(result1, value1);
}

TEST_F(WiredTigerDataStoreTest, DeleteTest) {
  // Arrange
  auto session_ = datastore_->new_datastore_session();
  EXPECT_NE(session_, nullptr);
  const char* key1 = "key1";
  const char* value1 = "value1";
  session_->upsert_kv(key1, value1);
  EXPECT_EQ(strcmp(session_->get(key1), value1), 0);

  // Act
  session_->delete_kv(key1);
  const char* result1 = session_->get(key1);

  // Assert
  EXPECT_EQ(result1, nullptr);
}