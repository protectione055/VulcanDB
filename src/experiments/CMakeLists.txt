cmake_minimum_required(VERSION 3.10)

include_directories(../../third_party/)

##############################################
#          BIM轻量化算法对比实验              #
##############################################
file(GLOB_RECURSE EXP_COMMON ./*.cpp)
file(GLOB_RECURSE IFC_COMPRESS_SOURCES ./compression_benchmark/*.cpp)
add_executable(ifc-compression-benchmark ${IFC_COMPRESS_SOURCES} ${EXP_COMMON})


find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
	include_directories(${ZLIB_INCLUDE_DIRS})
	target_link_libraries(ifc-compression-benchmark ${ZLIB_LIBRARIES})
endif()

target_link_libraries(ifc-compression-benchmark yaml-cpp)
target_link_libraries(ifc-compression-benchmark vulcan_core)