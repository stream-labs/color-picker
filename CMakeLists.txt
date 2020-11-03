cmake_minimum_required(VERSION 3.9)
project(color_picker)

# Settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

SET(NODEJS_URL "https://atom.io/download/atom-shell" CACHE STRING "Node.JS URL")
SET(NODEJS_NAME "iojs" CACHE STRING "Node.JS Name")
SET(NODEJS_VERSION "v9.3.3" CACHE STRING "Node.JS Version")

include(NodeJS)

nodejs_init()

# Source, Libraries and Directories
set(
	PROJECT_SOURCE 
	"${PROJECT_SOURCE_DIR}/source/color-picker-win.cpp" 
	"${PROJECT_SOURCE_DIR}/source/color-picker-win.h"
	"${PROJECT_SOURCE_DIR}/source/util-win.cpp" 
	"${PROJECT_SOURCE_DIR}/source/util-win.h"
	"${PROJECT_SOURCE_DIR}/res/resource.rc"
)

# Building

add_nodejs_module(${PROJECT_NAME} ${PROJECT_SOURCE} )

set_target_properties( ${PROJECT_NAME} PROPERTIES PREFIX "" SUFFIX ".node" )

target_compile_definitions(
	${PROJECT_NAME}
	PRIVATE
	-D_CRT_SECURE_NO_WARNINGS
	-DUNICODE
	-D_UNICODE)

target_compile_definitions(${PROJECT_NAME} PRIVATE BUILDING_NODE_EXTENSION)

# Include N-API wrappers
execute_process(COMMAND node -p "require('node-addon-api').include"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE NODE_ADDON_API_DIR
        )
string(REPLACE "\n" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
string(REPLACE "\"" "" NODE_ADDON_API_DIR ${NODE_ADDON_API_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${NODE_ADDON_API_DIR})

add_definitions(-DNAPI_VERSION=4)

# Distribute

install(
	TARGETS ${PROJECT_NAME}
	RUNTIME DESTINATION "./" COMPONENT Runtime
	LIBRARY DESTINATION "./" COMPONENT Runtime
)

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