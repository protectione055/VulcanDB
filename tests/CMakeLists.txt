include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/test)

file(GLOB_RECURSE TEST_DATASTORE_SOURCES ./storage/*.cpp ./storage/*/*.cpp)
add_executable(vulcan-test-storage ${TEST_DATASTORE_SOURCES})
target_link_libraries(vulcan-test-storage PRIVATE gtest gtest_main vulcan_core)
add_test(NAME vulcan-test-storage COMMAND vulcan-test-storage)
set_tests_properties(vulcan-test-storage vulcan-test-storage PROPERTIES LABELS storage)

file(GLOB_RECURSE TEST_BACKEND_SOURCES ./backend/*.cpp ./backend/*/*.cpp ${CMAKE_SOURCE_DIR}/src/backend/vulcan_param.cpp)
add_executable(vulcan-test-backend ${TEST_BACKEND_SOURCES})
target_link_libraries(vulcan-test-backend PRIVATE gtest gtest_main vulcan_core)
add_test(NAME vulcan-test-backend COMMAND vulcan-test-backend)
set_tests_properties(vulcan-test-backend vulcan-test-backend PROPERTIES LABELS backend)