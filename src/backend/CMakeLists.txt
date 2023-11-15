file(GLOB CMDLINE_SOURCES *.cpp)
add_executable(vulcan_cmdline ${CMDLINE_SOURCES})
target_link_libraries(vulcan_cmdline vulcan_core)