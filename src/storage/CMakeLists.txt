file(GLOB VULCAN_STORAGE_SOURCES *.cpp ./*/*.cpp)

add_library(vulcan_storage STATIC ${VULCAN_STORAGE_SOURCES})