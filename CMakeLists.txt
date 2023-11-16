cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
project(ifc-compression-benchmark VERSION 0.1.0 LANGUAGES C CXX)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

include(CPack)
include(CTest)
include(GNUInstallDirs)
enable_testing()

include_directories(src)

###############################################################
#   设置构建选项
###############################################################
if(NOT IS_ABSOLUTE ${INCLUDEDIR})
    set(INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR})
endif()
message(STATUS "INCLUDEDIR: ${INCLUDEDIR}")

set(STORAGE_ENGINE "WiredTiger" CACHE STRING "The storage engine to use")


###############################################################
#   设置编译选项
###############################################################
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O2")



###############################################################
#   构建VULCAN_CORE
###############################################################
set(SUBMODULES common storage ifcparse)
set(VULCAN_CORE_SRC)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
foreach(SUB_MODULE ${SUBMODULES})
    set(SUB_MODULE_DIR ${SRC_DIR}/${SUB_MODULE})
	if (EXISTS ${SUB_MODULE_DIR})
		file(GLOB_RECURSE SUBMODULE_SRC ${SRC_DIR}/${SUB_MODULE}/*.cpp)
		list(APPEND VULCAN_CORE_SRC ${SUBMODULE_SRC})
	else()
		message(FATAL_ERROR "Not found CMakeLists.txt in ${SRC_DIR}/${SUB_MODULE}")
	endif()
endforeach()
add_library(vulcan_core SHARED ${VULCAN_CORE_SRC})
add_subdirectory(${SRC_DIR}/compress_benchmark)


###############################################################
#   构建第三方依赖
###############################################################

add_subdirectory(third_party/yaml-cpp)

find_package(Boost REQUIRED)
if(NOT Boost_FOUND)
	message(FATAL_ERROR "Not found boost library")
endif()
target_link_libraries(vulcan_core ${Boost_LIBRARIES})

find_library(TCMALLOC_LIBRARY NAMES tcmalloc)
if(NOT TCMALLOC_LIBRARY)
    message(FATAL_ERROR "tcmalloc library not found")
endif()
target_link_libraries(vulcan_core ${TCMALLOC_LIBRARY})

find_package(spdlog REQUIRED)
if(NOT spdlog_FOUND)
	message(FATAL_ERROR "Not found spdlog package")
endif()
target_link_libraries(vulcan_core spdlog::spdlog)

if(${STORAGE_ENGINE} STREQUAL "WiredTiger")
  find_library(WIREDTIGER_LIB wiredtiger)
	if(NOT WIREDTIGER_LIB)
		message(FATAL_ERROR "Not found wiredtiger package")
	else()
	message("found wiredtiger library ${WIREDTIGER_LIB}")
	target_link_libraries(vulcan_core ${WIREDTIGER_LIB})
	endif()
endif()
