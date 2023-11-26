// Copyright 2023 VulcanDB
#pragma once

#include <map>
#include <string>
#include <vector>

#include "backend/seda/seda_config.h"
#include "backend/seda/seda_defs.h"
#include "backend/seda/session_stage.h"
#include "backend/seda/stage.h"
#include "backend/seda/thread_pool.h"
#include "common/ini_parser.h"
#include "common/string.h"

namespace vulcan {

/**
 * @brief The Seda class represents a configuration for the SEDA (Staged
 * Event-Driven Architecture) framework.
 *
 * SEDA is a software architecture pattern that divides an application into a
 * set of stages, where each stage processes events asynchronously. This class
 * provides a map-based configuration for SEDA, allowing users to retrieve
 * configuration values for different sections.
 */
class Seda {
 public:
  Seda() = default;
  ~Seda() = default;

  /**
   * @brief Retrieves the configuration values for a specific section.
   *
   * @param section The name of the section to retrieve configuration values
   * for.
   * @return A map containing the configuration values for the specified
   * section. If the section does not exist, an empty map is returned.
   */
  std::map<std::string, std::string> get(const std::string &section) {
    if (seda_.find(section) == seda_.end()) {
      return std::map<std::string, std::string>();
    }
    return seda_[section];
  }

  std::string get(const std::string &key, const std::string &defaultValue,
                  const std::string &section) {
    if (seda_.find(section) == seda_.end()) {
      return defaultValue;
    }
    return seda_[section][key];
  }

  static const char CFG_DELIMIT_TAG = ',';

 private:
  std::map<std::string, std::map<std::string, std::string>> seda_ = {
      {SEDA_BASE_NAME,
       {{"EventHistory", "false"},
        {"MaxEventHistoryNum", "100"},
        {THREAD_POOLS_NAME, "SQLThreads,IOThreads,DefaultThreads"},
        {"STAGES", "SessionStage"}}},
      {"SQLThreads", {{"count", "3"}}},
      {"IOThreads", {{"count", "3"}}},
      {DEFAULT_THREAD_POOL, {{COUNT, "3"}}},
      {SESSION_STAGE_NAME, {{THREAD_POOL_ID, "SQLThreads"}}}};
};

/**
 *  A class to configure seda stages
 *  Each application uses an ini file to define the stages that make up the
 *  application, the threadpool that the stages use, and the parameters that
 *  are passed to configure each individual stage.  The SedaConfig class
 *  consumes this xml file, parses it, and instantiates the indicated
 *  configuration.  It also then provides access to individual stages_ and
 *  threadpools within the configuration. The parameters passed to each
 *  stage consists of the global attributes defined for all the seda stages_
 *  in the seda instance as well as the attributes defined for that specific
 *  stage. The attributes defined for each stage will override the global
 *  attributes in case of duplicate attributes
 */
class SedaConfig {
 public:
  typedef enum { SUCCESS = 0, INITFAIL, PARSEFAIL } status_t;

  static SedaConfig *&get_instance();

  /**
   * Destructor
   * @post configuration is deleted
   */
  ~SedaConfig();

  /**
   * start the parsed, instantiated configuration
   * @pre   configuration parsed and instantiated
   * @post  if SUCCESS, the SEDA pipleine is now running.
   *
   */
  status_t start();

  /**
   * Complete Initialization of the mThreadPools and stages_
   * Use the parsed config to initialize the required mThreadPools and
   * stages_, and start them running.  If the config has not yet been
   * parsed then try to parse it first.  The init function combines
   * parse(), instantiate() and start()
   *
   * @pre empty mThreadPools and stages_
   * @post if returns SUCCESS then
   *          mThreadPools and stages_ created/initialized and running
   * @post if returns INITFAIL or PARSEFAIL then
   *          mThreadPools and stage list are empty
   */
  status_t init();

  /**
   * Clean-up the threadpool and stages_
   * @post all stages_ disconnected and deleted, all mThreadPools deleted
   */
  void cleanup();

  /**
   * get the desired stage given a string
   *
   * @param[in] stagename   take in the stage name and convert it to a Stage
   * @pre
   * @return a reference to the Stage
   */
  Stage *get_stage(const char *stagename);

  /**
   * get the desired threadpool a string
   *
   * @param[in] index   take in the index for threadpool
   * @pre
   * @return a reference to the ThreadPool
   */
  Threadpool &get_thread_pool(const int index);

  /**
   * Get a list of all stage names
   * @param[in/out] names   names of all stages_
   */
  void get_stage_names(std::vector<std::string> &names) const;

  /**
   * Query the number of queued events at each stage.
   * @param[in/out] stats   number of events enqueued at each
   *   stage.
   */
  void get_stage_queue_status(std::vector<int> &stats) const;

  std::map<std::string, Stage *>::iterator begin();
  std::map<std::string, Stage *>::iterator end();

 private:
  // Constructor
  SedaConfig();

  /**
   * instantiate the mThreadPools and stages_
   * Instantiate the mThreadPools and stages_ defined in the configuration
   *
   * @pre  cfg_ptr is not NULL
   * @post returns SUCCESS ==> all mThreadPools and stages_ are created
   *       returns INITFAIL ==> mThreadPools and stages_ are deleted
   */
  status_t instantiate();

  status_t init_thread_pool();

  std::string get_thread_pool(std::string &stage_name);

  status_t init_stages();
  status_t gen_next_stages();

  /**
   * delete all mThreadPools and stages_
   * @pre  all existing stages_ are disconnected
   * @post all mThreadPools and stages_ are deleted
   */
  void clear_config();

  SedaConfig &operator=(const SedaConfig &cevtout);

  static SedaConfig *instance_;

  // In old logic, SedaConfig will parse seda configure file
  // but here, only one configure file
  std::string cfg_file_;
  std::string cfg_str_;

  std::map<std::string, Threadpool *>
      thread_pools_;                       // stage_name -> threadpool
  std::map<std::string, Stage *> stages_;  // stage_name -> stage
  std::vector<std::string> stage_names_;
  Seda seda_cfg_;
};

inline std::map<std::string, Stage *>::iterator SedaConfig::begin() {
  return stages_.begin();
}

inline std::map<std::string, Stage *>::iterator SedaConfig::end() {
  return stages_.end();
}

inline Stage *SedaConfig::get_stage(const char *stagename) {
  if (stagename) {
    std::string sname(stagename);
    return stages_[stagename];
  }
  return nullptr;
}

// Global seda config object
SedaConfig *&get_seda_config();

bool &get_event_history_flag();
uint32_t &get_max_event_hops();

}  // namespace vulcan
