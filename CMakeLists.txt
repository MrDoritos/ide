cmake_minimum_required(VERSION 3.7.2)
set (PROJECT_NAME ide)
project (${PROJECT_NAME} VERSION 1.0)

FILE(GLOB SOURCE_FILES src/*.cpp)
FILE(GLOB UI_FILES ../console-ui/*.cpp)

#add_subdirectory(../console)
#add_subdirectory(../console-ui)
#include_directories("H:/cpp-projects/console")
#include_directories("H:/cpp-projects/console-ui")
set(CONSOLE_INCLUDE_PATH "../console")
set(CONSOLEUI_INCLUDE_PATH "../console-ui")
include_directories(${CONSOLE_INCLUDE_PATH})
include_directories(${CONSOLEUI_INCLUDE_PATH})

add_definitions(-w)
add_definitions(-fpermissive)
add_definitions(-ggdb)

if(MINGW)
include_directories(src/windows)
add_executable(
		${PROJECT_NAME} 
		${SOURCE_FILES}
		${UI_FILES}
		../console/advancedConsole.cpp
		../console/console.windows.cpp
		)
add_definitions(-municode)
add_definitions(-D__WIN32)
endif()

if(UNIX)
add_executable(
		${PROJECT_NAME} 
		${SOURCE_FILES}
		${UI_FILES}
		../console/advancedConsole.cpp
		../console/console.linux.cpp
		)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
			"${PROJECT_BINARY_DIR}"
			)

set (CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_STANDARD_REQUIRED True)
set (CMAKE_CXX_COMPILER g++)

if(UNIX)
	target_link_libraries(${PROJECT_NAME} ncursesw pthread)
endif()

if(MINGW)
	target_link_libraries(${PROJECT_NAME})
endif()
