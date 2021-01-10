cmake_minimum_required(VERSION 3.1)

set(PROJECT ImGuiFileDialog)

enable_language(CXX)
project(${PROJECT} CXX)

if(WIN32)
    set(DIRENT_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/dirent/include)
    include_directories(${DIRENT_INCLUDE_DIR})
endif()

if(MSVC)
    # Ignore 4055 for glad
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /IGNORE:4055")
    add_definitions(-D_USE_MATH_DEFINES)
    add_definitions(-DMSVC)
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -std=c++11")
endif ()

add_definitions(-DIMGUI_DISABLE_OBSOLETE_FUNCTIONS)

file(GLOB IMGUIFILEDIALOG_SOURCES 
		${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/*.h)
source_group(src FILES ${IMGUIFILEDIALOG_SOURCES})

add_library(${PROJECT} STATIC ${IMGUIFILEDIALOG_SOURCES})

set(IMGUIFILEDIALOG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(IMGUIFILEDIALOG_LIBRARIES ${IMGUIFILEDIALOG_LIBRARIES} ImGuiFileDialog)
