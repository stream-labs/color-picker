cmake_minimum_required(VERSION 3.1)
project(color_picker)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Settings

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(NODEJS_URL "https://atom.io/download/atom-shell" CACHE STRING "Node.JS URL")
#set(NODEJS_NAME "iojs" CACHE STRING "Node.JS Name")
#set(NODEJS_VERSION "v6.1.11" CACHE STRING "Node.JS Version")

#include(NodeJS)
#include(CPack)

#nodejs_init()

# Source, Libraries and Directories

if(WIN32)
	set(
		PROJECT_SOURCE 
		"${PROJECT_SOURCE_DIR}/source/color-picker-win.cpp" 
		"${PROJECT_SOURCE_DIR}/source/color-picker-win.h"
		"${PROJECT_SOURCE_DIR}/source/util-win.cpp" 
		"${PROJECT_SOURCE_DIR}/source/util-win.h"
		"${PROJECT_SOURCE_DIR}/res/resource.rc"
	)
endif()

set(
	PROJECT_LIBRARIES
	${NODEJS_LIBRARIES}
)

if(WIN32)
	SET(PROJECT_INCLUDE_PATHS
		${NODEJS_INCLUDE_DIRS}
		"${CMAKE_SOURCE_DIR}/source/"
	)
endif()

# Building
include_directories(${CMAKE_JS_INC})

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCE} ${CMAKE_JS_SRC})
#add_nodejs_module( ${PROJECT_NAME} ${PROJECT_SOURCE})

if(WIN32)
	target_link_libraries(
		${PROJECT_NAME}
		${PROJECT_LIBRARIES}
	)
endif()

target_compile_definitions(
	${PROJECT_NAME}
	PRIVATE BUILDING_NODE_EXTENSION
)

set(
	CompilerFlags
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

if(WIN32)
	target_compile_definitions(
		${PROJECT_NAME}
		PRIVATE
		-D_CRT_SECURE_NO_WARNINGS
		-DUNICODE
		-D_UNICODE)
endif()

# Include N-API wrappers
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_JS_INC})
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/node_modules/node-addon-api")
target_link_libraries(${PROJECT_NAME} ${CMAKE_JS_LIB})

add_definitions(-DNAPI_VERSION=4)

set_target_properties(
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