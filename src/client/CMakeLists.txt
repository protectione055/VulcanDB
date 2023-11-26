include_directories(${CMAKE_SOURCE_DIR}/src/)

###############################################################
#   构建VULCAN_CLIENT
###############################################################
file(GLOB_RECURSE CLIENT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(vulcan-client ${CLIENT_SRC})
target_link_libraries(vulcan-client vulcan_core)

###############################################################
#   安装到指定目录
###############################################################
add_custom_command(TARGET vulcan-client POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:vulcan-client> ${CMAKE_SOURCE_DIR}/build/bin/)