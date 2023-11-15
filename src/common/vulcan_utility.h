// Copyright 2023 Ziming Zhang
#pragma once

#include <filesystem>
#include <stdexcept>
#include <string>

namespace vulcan {

// 创建并返回一个临时目录
// @return 临时目录路径
std::filesystem::path create_temp_directory();

// 将windows下的换行符替换为unix下的换行符
// @param file 转换的文件路径
void replace_win_newlines_to_unix(std::string fileresult_file,
                                  std::string new_file = "");

// 将字符串中的子串替换为新的子串
// @param resource_str 源字符串
// @param sub_str 要替换的子串
// @param new_str 新子串
// @return 替换后的字符串
std::string subreplace(std::string resource_str, std::string sub_str,
                       std::string new_str);

}  // namespace vulcan
