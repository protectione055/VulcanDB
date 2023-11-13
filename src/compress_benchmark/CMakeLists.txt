cmake_minimum_required(VERSION 3.10)

include_directories(include)
include_directories(../../third_party/)
file(GLOB_RECURSE BENCHMARK_SOURCES ${CMAKE_SOURCE_DIR}/src/compress_benchmark/*.cpp)
add_executable(ifc-compression-benchmark ${BENCHMARK_SOURCES})


find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIRS})
	target_link_libraries(ifc-compression-benchmark ${ZLIB_LIBRARIES})
endif()

target_link_libraries(ifc-compression-benchmark yaml-cpp)