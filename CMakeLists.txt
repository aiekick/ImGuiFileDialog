cmake_minimum_required(VERSION 3.15)
project(ImGuiFileDialog)

find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(gl3w CONFIG REQUIRED)

if(UNIX)
    if (APPLE)
        add_definitions(-DAPPLE)
    else()
        add_definitions(-DLINUX)
    endif()
    add_definitions(-DMINGW32)
elseif(MSVC)
    add_definitions(-DWIN32)
endif()

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(dirent)
include_directories(${GLFW3_INCLUDE_DIR})
include_directories(libs/gl3w)
include_directories(libs/glfw/include)

add_executable(ImGuiFileDialog
        dirent/dirent.h
        imconfig.h
        imgui.cpp
        imgui.h
        imgui_demo.cpp
        imgui_draw.cpp
        imgui_impl_glfw.cpp
        imgui_impl_glfw.h
        imgui_impl_opengl3.cpp
        imgui_impl_opengl3.h
        imgui_internal.h
        imgui_widgets.cpp
        ImGuiFileDialog.cpp
        ImGuiFileDialog.h
        imstb_rectpack.h
        imstb_textedit.h
        imstb_truetype.h
        main.cpp
        stb_rect_pack.h
        stb_textedit.h
        stb_truetype.h)

target_link_libraries(ImGuiFileDialog PRIVATE glfw unofficial::gl3w::gl3w "${OPENGL_LIBRARIES}" "${CMAKE_THREAD_LIBS_INIT}")
