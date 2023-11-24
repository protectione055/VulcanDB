include_directories(${CMAKE_SOURCE_DIR}/src)

file(GLOB_RECURSE TEST_SOURCES ./*.cpp ./*/*.cpp)
add_executable(test-wiredtiger_datastore ${TEST_SOURCES})
target_link_libraries(test-wiredtiger_datastore PRIVATE gtest gtest_main vulcan_core)
add_test(NAME test-wiredtiger_datastore COMMAND test-wiredtiger_datastore)
set_tests_properties(test-wiredtiger_datastore test-wiredtiger_datastore PROPERTIES LABELS datastore)