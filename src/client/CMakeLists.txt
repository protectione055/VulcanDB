include_directories(${CMAKE_SOURCE_DIR}/src/)

###############################################################
#   构建VULCAN_CLIENT
###############################################################
file(GLOB_RECURSE CLIENT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(vulcql ${CLIENT_SRC})
target_link_libraries(vulcql vulcan_core)
