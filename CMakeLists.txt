cmake_minimum_required(VERSION 3.1)

project(ImGuiFileDialog)

add_library(ImGuiFileDialog STATIC
    ImGuiFileDialog.cpp
    ImGuiFileDialog.h
    ImGuiFileDialogConfig.h
)

target_include_directories(ImGuiFileDialog PUBLIC 
	${CMAKE_CURRENT_SOURCE_DIR})
