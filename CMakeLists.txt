cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
project(ifc-compression-benchmark VERSION 0.1.0 LANGUAGES C CXX)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

include(CPack)
include(CTest)
enable_testing()

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -O2")
set(CMAKE_BUILD_TYPE Debug)


# 添加目标执行文件
include_directories(include)
include_directories(third_party/)
file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(ifc-compression-benchmark ${SOURCES})

# 添加链接库
find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIRS})
	target_link_libraries(ifc-compression-benchmark ${ZLIB_LIBRARIES})
endif()

add_subdirectory("third_party/yaml-cpp")
target_link_libraries(ifc-compression-benchmark yaml-cpp)