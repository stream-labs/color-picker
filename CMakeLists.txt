cmake_minimum_required(VERSION 3.9)
project(color_picker)

# Settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

SET(NODEJS_URL "https://atom.io/download/atom-shell" CACHE STRING "Node.JS URL")
SET(NODEJS_NAME "iojs" CACHE STRING "Node.JS Name")
SET(NODEJS_VERSION "v13.4.0" CACHE STRING "Node.JS Version")

include(NodeJS)

nodejs_init()

SET(PROJECT_INCLUDE_PATHS ${NODEJS_INCLUDE_DIRS} "${CMAKE_SOURCE_DIR}/source/")

# Source, Libraries and Directories
set(
	PROJECT_SOURCE 
	"${PROJECT_SOURCE_DIR}/source/color-picker-win.cpp" 
	"${PROJECT_SOURCE_DIR}/source/color-picker-win.h"
	"${PROJECT_SOURCE_DIR}/source/util-win.cpp" 
	"${PROJECT_SOURCE_DIR}/source/util-win.h"
	"${PROJECT_SOURCE_DIR}/res/resource.rc"
)
# Include N-API wrappers
execute_process(COMMAND node -p "require('node-addon-api').include"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE NODE_ADDON_API_DIR
        )
string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})

list(APPEND PROJECT_INCLUDE_PATHS ${NODE_ADDON_API_DIR})

add_definitions(-DNAPI_VERSION=7)

# Building

add_nodejs_module(${PROJECT_NAME} ${PROJECT_SOURCE} )

target_link_libraries(${PROJECT_NAME} ${PROJECT_LIBRARIES})
target_include_directories(${PROJECT_NAME} PUBLIC  ${PROJECT_INCLUDE_PATHS})
set_property (TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17 )

target_compile_definitions(${PROJECT_NAME} PRIVATE -DUNICODE -D_UNICODE BUILDING_NODE_EXTENSION -D_CRT_SECURE_NO_WARNINGS)

set(CompilerFlags
	CMAKE_CXX_FLAGS
	CMAKE_CXX_FLAGS_DEBUG
	CMAKE_CXX_FLAGS_RELEASE
	CMAKE_CXX_FLAGS_RELWITHDEBINFO
	CMAKE_C_FLAGS
	CMAKE_C_FLAGS_DEBUG
	CMAKE_C_FLAGS_RELEASE
	CMAKE_C_FLAGS_RELWITHDEBINFO
	)
foreach(CompilerFlag ${CompilerFlags})
  string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
endforeach()

SET_TARGET_PROPERTIES(
	${PROJECT_NAME}
	PROPERTIES
	PREFIX ""
	SUFFIX ".node"
)
# Distribute

install(
	TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION "./" COMPONENT Runtime
	LIBRARY DESTINATION "./" COMPONENT Runtime
)

if(WIN32)
	include(FetchContent)

	# Compare current linked libs with prev
	FetchContent_Declare(deps_checker URL "https://raw.githubusercontent.com/stream-labs/obs-studio-node/staging/dependency_checker/check_dependencies.cmd" DOWNLOAD_NO_EXTRACT true)
	FetchContent_MakeAvailable(deps_checker)

	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${deps_checker_SOURCE_DIR}/check_dependencies.cmd $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} $<CONFIG> )
endif()

if(WIN32)
	INSTALL(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> DESTINATION ./ OPTIONAL)
endif()

# JavaScript

install(
	FILES "${CMAKE_SOURCE_DIR}/package.json"
	DESTINATION "./"
)

install(
	FILES "${CMAKE_SOURCE_DIR}/main.js"
	DESTINATION "./"
)