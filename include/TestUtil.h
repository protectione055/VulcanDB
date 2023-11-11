#pragma once

#include <filesystem>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace compbench {

/*接受一个转换函数作为参数，递归遍历目录的每个文件，对每个输入流执行该函数并转换为一个输出流
 *
 * @param root_dir 遍历的根目录
 * @param output_dir 输出目录
 * @param origin_postfix 原始文件后缀
 * @param target_postfix 目标文件后缀
 * @param config 传递给convert_func的参数
 * @param convert_func
 * 对文件的处理函数，处理后的文件输出到output_dir，返回结果的iostream
 */
void traverseDir(
    const std::filesystem::path& root_dir,
    const std::filesystem::path& output_dir, std::string origin_postfix,
    std::string target_postfix,
    std::function<void(const std::string&, const std::string&)> convert_func);

}  // namespace compbench