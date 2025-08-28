cmake_minimum_required(VERSION 3.20)
project(ImGuiFileDialog)

add_library(ImGuiFileDialog STATIC
    ImGuiFileDialog.cpp
    ImGuiFileDialog.h
    ImGuiFileDialogConfig.h
)

# Use generator expressions for proper include directory handling
target_include_directories(ImGuiFileDialog
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)

if(UNIX)
    target_compile_options(ImGuiFileDialog PUBLIC -Wno-unknown-pragmas)
endif()

# Installation configuration
include(GNUInstallDirs)

# Install the library
install(TARGETS ImGuiFileDialog
    EXPORT ImGuiFileDialogTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install headers
install(FILES 
    ImGuiFileDialog.h 
    ImGuiFileDialogConfig.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install export targets
install(EXPORT ImGuiFileDialogTargets
    FILE ImGuiFileDialogConfig.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ImGuiFileDialog
)