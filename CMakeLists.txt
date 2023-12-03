cmake_minimum_required(VERSION 3.5)

project(ImGuiFileDialog)

set(IMGUI_FILEDIALOG_CONFIG "ImGuiFileDialogConfig.h" CACHE FILEPATH "Change this to supply a custom ImGuiFileDialogConfig.h")

add_library(ImGuiFileDialog STATIC
    ImGuiFileDialog.cpp
    ImGuiFileDialog.h
    ${IMGUI_FILEDIALOG_CONFIG}
)

target_include_directories(ImGuiFileDialog PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(ImGuiFileDialog PRIVATE CUSTOM_IMGUIFILEDIALOG_CONFIG="${IMGUI_FILEDIALOG_CONFIG}")

if(UNIX)
    target_compile_options(ImGuiFileDialog PUBLIC "-Wno-unknown-pragmas")
endif()
