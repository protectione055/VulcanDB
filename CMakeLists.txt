cmake_minimum_required(VERSION 3.0.0)
project(VulcanDB VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

include_directories(src)
include_directories(third_party/)

###############################################################
#   设置构建选项
###############################################################
if(NOT IS_ABSOLUTE ${INCLUDEDIR})
    set(INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR})
endif()
message(STATUS "INCLUDEDIR: ${INCLUDEDIR}")

option(BUILD_TESTING "Build tests" ON)
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
set(VULCAN_CORE_MODULES
	"common"
	"ifcparse"
	"storage"
	"execution"
)
set(SUBMODULES ${VULCAN_CORE_MODULES})
set(VULCAN_CORE_SRC)
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
foreach(SUB_MODULE ${SUBMODULES})
    set(SUB_MODULE_DIR ${SRC_DIR}/${SUB_MODULE})
	if (EXISTS ${SUB_MODULE_DIR})
		file(GLOB_RECURSE SUBMODULE_SRC ${SUB_MODULE_DIR}/*.cpp ${SUB_MODULE_DIR}/*/*.cpp)
		list(APPEND VULCAN_CORE_SRC ${SUBMODULE_SRC})
	else()
		message(FATAL_ERROR "Not found CMakeLists.txt in ${SRC_DIR}/${SUB_MODULE}")
	endif()
endforeach()
add_library(vulcan_core SHARED ${VULCAN_CORE_SRC})
add_subdirectory(${SRC_DIR}/experiments)

###############################################################
#   构建vulcan-server
###############################################################
file(GLOB_RECURSE vulcan-server_SRC ${SRC_DIR}/backend/*.cpp)
add_executable(vulcan-server ${vulcan-server_SRC})
target_link_libraries(vulcan-server vulcan_core)

###############################################################
#   构建VULCAN_CLIENT
###############################################################
add_subdirectory(${SRC_DIR}/client)

###############################################################
#   构建TESTS
###############################################################
if(BUILD_TESTING)
message("Build tests")
include(CPack)
include(CTest)
include(GNUInstallDirs)
enable_testing()
add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/googletest EXCLUDE_FROM_ALL)
add_subdirectory(tests)
else()
message("Not build tests")
endif()

###############################################################
#   构建第三方依赖
###############################################################

add_subdirectory(third_party/yaml-cpp EXCLUDE_FROM_ALL)

add_subdirectory(third_party/libevent EXCLUDE_FROM_ALL)
target_link_libraries(vulcan_core event_pthreads event)

find_package(fmt REQUIRED)
if(fmt_FOUND)
	target_link_libraries(vulcan_core fmt::fmt)
else()
	message("Not found fmt package, using third_party fmt")
	add_subdirectory(third_party/fmt EXCLUDE_FROM_ALL)
	target_link_libraries(vulcan_core fmt)
endif()

find_package(Boost REQUIRED)
if(NOT Boost_FOUND)
	message(FATAL_ERROR "Not found boost library")
endif()
target_link_libraries(vulcan_core ${Boost_LIBRARIES})

target_link_libraries(vulcan_core -ltcmalloc)

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

###############################################################
#   安装
###############################################################
# install(TARGETS vulcan_core vulcan-server
# 	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
# 	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
# 	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
add_custom_command(TARGET vulcan-server POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:vulcan-server> ${CMAKE_SOURCE_DIR}/build/bin/)