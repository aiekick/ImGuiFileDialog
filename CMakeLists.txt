cmake_minimum_required(VERSION 3.20)
project(ImGuiFileDialog)

# Option to control installation (disabled by default if used as subproject)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(IGFD_INSTALL_DEFAULT ON)
else()
    set(IGFD_INSTALL_DEFAULT OFF)
endif()

option(IGFD_INSTALL "Install ImGuiFileDialog library and headers" ${IGFD_INSTALL_DEFAULT})

find_package(imgui QUIET) # fails quietly if not found (imgui is the target name for vcpkg and nixpkgs).

add_library(ImGuiFileDialog
    ImGuiFileDialog.cpp
    ImGuiFileDialog.h
    ImGuiFileDialogConfig.h
)

if(imgui_FOUND)
    target_link_libraries(ImGuiFileDialog PUBLIC imgui::imgui)
    message(STATUS "ImGuiFileDialog: Found imgui package, linking automatically")
else()
    message(STATUS "ImGuiFileDialog: imgui package not found - ensure ImGui is available in your project")
endif()

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
if(IGFD_INSTALL)
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
       NAMESPACE ImGuiFileDialog::
       DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ImGuiFileDialog
    )
endif()