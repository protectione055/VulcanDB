// Copyright 2023 VulcanDB
#pragma once

#include <stdio.h>

#include <iostream>
#include <map>
#include <set>
#include <string>

namespace vulcan {

//********************************************************************

/**
 * @brief The IniFile class represents a configuration parser for INI files.
 *
 * This class provides methods to load, retrieve, modify, and output INI
configurations.
 * It supports loading multiple INI configuration files and provides
thread-unsafe access to the data.
 *
 * Usage:
 * 1. Create an instance of IniFile.
 * 2. Load INI configuration using the load() method.
 * 3. Retrieve configuration values using the get() method.
 * 4. Modify configuration values using the put() method.
 * 5. Output the entire configuration to a string using the to_string() method.
 *
 * Note: This class does not support concurrent modification of the data.
 *
 * @example
 * [section]
 * VARNAME=VALUE
 */
class IniFile {
 public:
  /**
   * To simplify the logic, no lock's when loading configuration
   * So don't modify the data parallel
   */
  IniFile();
  ~IniFile();

  /**
   * load one ini configuration
   * it support load multiple ini configuration files
   * @return, 0 means success, others means failed
   */
  int load(const std::string &ini_file);

  /**
   * get the map of the section
   * if the section doesn't exist, return one empty section
   */
  const std::map<std::string, std::string> &get(
      const std::string &section = DEFAULT_SECTION);

  /**
   * get the value of the key in the section,
   * if the key-value doesn't exist,
   * use the input default_value
   */
  std::string get(const std::string &key, const std::string &default_value,
                  const std::string &section = DEFAULT_SECTION);

  /**
   * put the key-value pair to the section
   * if the key-value already exist, just replace it
   * if the section doesn't exist, it will create this section
   */
  int put(const std::string &key, const std::string &value,
          const std::string &section = DEFAULT_SECTION);

  /**
   * output all configuration to one string
   */
  void to_string(std::string &output_str);

  static const std::string DEFAULT_SECTION;

  // one line max length
  static const int MAX_CFG_LINE_LEN = 1024;

  // value split tag
  static const char CFG_DELIMIT_TAG = ',';

  // comments's tag
  static const char CFG_COMMENT_TAG = '#';

  // continue line tag
  static const char CFG_CONTINUE_TAG = '\\';

  // session name tag
  static const char CFG_SESSION_START_TAG = '[';
  static const char CFG_SESSION_END_TAG = ']';

 protected:
  /**
   * insert one empty session to sections_
   */
  void insert_session(const std::string &session_name);

  /**
   * switch session according to the session_name
   * if the section doesn't exist, it will create one
   */
  std::map<std::string, std::string> *switch_session(
      const std::string &session_name);

  /**
   * insert one entry to session_map
   * line's format is "key=value"
   *
   */
  int insert_entry(std::map<std::string, std::string> *session_map,
                   const std::string &line);

  typedef std::map<std::string, std::map<std::string, std::string>> SessionsMap;

 private:
  static const std::map<std::string, std::string> empty_map_;

  std::set<std::string> file_names_;
  SessionsMap sections_;
};

/**
 * Global configurate propertis
 */
IniFile *&get_properties();
//********************************************************************

}  // namespace vulcan
