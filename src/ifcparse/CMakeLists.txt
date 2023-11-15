# 配置IfcOpenShell
option(IFCXML_SUPPORT "Build vulcan_ifcparse with ifcXML support (requires libxml2)." ON)

function(files_for_ifc_version IFC_VERSION RESULT_NAME)
    set(IFC_PARSE_DIR ${CMAKE_SOURCE_DIR}/src/ifcparse)
    set(${RESULT_NAME}
        ${IFC_PARSE_DIR}/Ifc${IFC_VERSION}.h
        ${IFC_PARSE_DIR}/Ifc${IFC_VERSION}enum.h
        ${IFC_PARSE_DIR}/Ifc${IFC_VERSION}.cpp
        PARENT_SCOPE
    )
endfunction()

set(IFCOPENSHELL_LIBRARIES vulcan_ifcparse)

# vulcan_ifcparse
file(GLOB IFCPARSE_H_FILES_ALL ${IFC_PARSE_DIR}/*.h)
file(GLOB IFCPARSE_CPP_FILES_ALL ${IFC_PARSE_DIR}/*.cpp)

foreach(file ${IFCPARSE_H_FILES_ALL})
    get_filename_component(filename "${file}" NAME)

    if(NOT "${filename}" MATCHES "[0-9]")
        list(APPEND IFCPARSE_H_FILES "${file}")
    endif()
endforeach()

foreach(file ${IFCPARSE_CPP_FILES_ALL})
    get_filename_component(filename "${file}" NAME)

    if(NOT "${filename}" MATCHES "[0-9]")
        list(APPEND IFCPARSE_CPP_FILES "${file}")
    endif()
endforeach()

foreach(schema ${SCHEMA_VERSIONS})
    list(APPEND IFCPARSE_H_FILES
        ${IFC_PARSE_DIR}/Ifc${schema}.h
        ${IFC_PARSE_DIR}/Ifc${schema}-definitions.h
    )
    list(APPEND IFCPARSE_CPP_FILES
        ${IFC_PARSE_DIR}/Ifc${schema}.cpp
        ${IFC_PARSE_DIR}/Ifc${schema}-schema.cpp
    )
endforeach()

foreach(file ${IFCPARSE_H_FILES_ALL})
    get_filename_component(filename "${file}" NAME)

    if(NOT "${filename}" MATCHES "[0-9]")
        list(APPEND IFCPARSE_H_FILES "${file}")
    endif()
endforeach()

foreach(file ${IFCPARSE_CPP_FILES_ALL})
    get_filename_component(filename "${file}" NAME)

    if(NOT "${filename}" MATCHES "[0-9]")
        list(APPEND IFCPARSE_CPP_FILES "${file}")
    endif()
endforeach()

foreach(schema ${SCHEMA_VERSIONS})
    list(APPEND IFCPARSE_H_FILES
        ${IFC_PARSE_DIR}/Ifc${schema}.h
        ${IFC_PARSE_DIR}/Ifc${schema}-definitions.h
    )
    list(APPEND IFCPARSE_CPP_FILES
        ${IFC_PARSE_DIR}/Ifc${schema}.cpp
        ${IFC_PARSE_DIR}/Ifc${schema}-schema.cpp
    )
endforeach()

set(IFCPARSE_FILES ${IFCPARSE_H_FILES} ${IFCPARSE_CPP_FILES})
add_library(vulcan_ifcparse STATIC ${IFCPARSE_FILES})
set_target_properties(vulcan_ifcparse PROPERTIES COMPILE_FLAGS -DIFC_PARSE_EXPORTS VERSION "${PROJECT_VERSION}" SOVERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")

# set(BCRYPT_LIBRARIES "bcrypt.lib")

# find_package(BCrypt REQUIRED)
# if(BCRYPT_FOUND)
# 	include_directories(${BCRYPT_INCLUDE_DIRS})
# else()
# 	message(FATAL_ERROR "BCrypt not found")
# endif()
set(BCRYPT_LIBRARIES "bcrypt.lib")

find_package(Boost REQUIRED)
if(BOOST_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
else()
	message(FATAL_ERROR "Boost not found")
endif()

find_package(LibXml2 REQUIRED)
if(LIBXML2_FOUND)
	include_directories(${LIBXML2_INCLUDE_DIR})
else()
	message(FATAL_ERROR "LibXml2 not found")
endif()

target_link_libraries(vulcan_ifcparse ${BCRYPT_LIBRARIES} ${LIBXML2_LIBRARIES} ${Boost_LIBRARIES})

set(INCLUDEDIR src)
set(LIBDIR ${CMAKE_SOURCE_DIR}/build/lib)
# CMake installation targets
install(FILES ${IFCPARSE_H_FILES}
	DESTINATION ${INCLUDEDIR}/ifcparse
)

install(TARGETS vulcan_ifcparse
	ARCHIVE DESTINATION ${LIBDIR}
	LIBRARY DESTINATION ${LIBDIR}
	RUNTIME DESTINATION ${BINDIR}
)