cmake_policy(SET CMP0048 NEW)
cmake_minimum_required(VERSION 2.8)

project(ImGuiFileDialog)

# thanks to the cmake project for imgui available here : https://github.com/aid95/MinimalImGui

# 3rd party libraries
include(cmake/3rdparty.cmake)

# macro and configurations
include(cmake/config.cmake)

# projects
make_executable()

if(UNIX)
    if (APPLE)
        add_definitions(-DAPPLE)
    else()
        add_definitions(-DLINUX)
    endif()
    add_definitions(-DUNIX)
elseif(WIN32)
    add_definitions(-DWIN32)
    add_definitions(-DMINGW32)
    set(DIRENT_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/dirent/include)
    include_directories(${DIRENT_INCLUDE_DIR})
endif()
if(MSVC)
    add_definitions(-DMSVC)
endif()

add_definitions(-DCUSTOM_IMGUIFILEDIALOG_CONFIG="../CustomImGuiFileDialogConfig.h")

include_directories(
        ${PLOG_INCLUDE_DIR}
        ${GLM_INCLUDE_DIR}
        ${OPENGL_INCLUDE_DIR}
        ${GLFW_INCLUDE_DIR}
        ${GLAD_INCLUDE_DIR}
        ${IMGUI_INCLUDE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/ImGuiFileDialog
)

target_link_libraries(${PROJECT}
        ${OPENGL_LIBRARIES}
        ${GLFW_LIBRARIES}
        ${GLAD_LIBRARIES}
        ${IMGUI_LIBRARIES}
        )

