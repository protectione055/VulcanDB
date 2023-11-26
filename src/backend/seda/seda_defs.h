// Copyright 2023 VulcanDB
#pragma once

#include <map>
#include <string>

namespace vulcan {
#define SEDA_CFG_FILE "./seda.ini"  // seda配置文件

#define SEDA_BASE_NAME "SEDA_BASE"       // 配置文件中SEDA_BASE字段名
#define THREAD_POOLS_NAME "ThreadPools"  // 配置文件中ThreadPools字段名
#define STAGES "STAGES"                  // 配置文件中STAGES字段名
#define SESSION_STAGE_NAME "SessionStage"
#define COUNT "count"  // 配置文件中线程池大小count字段名
#define THREAD_POOL_ID "ThreadId"  // 配置文件中线程池id字段名
#define NEXT_STAGES "NextStages"   // 配置文件中下一个stage字段名
#define DEFAULT_THREAD_POOL "DefaultThreads"  // 配置文件中默认线程池大小字段名
}  // namespace vulcan
