cmake_minimum_required(VERSION 3.1)

set(PROJECT ImGuiFileDialog)
set(PROJECT_LIB ${PROJECT}_Lib)
set(PROJECT_APP ${PROJECT}_App)

enable_language(C CXX)
project(${PROJECT} CXX)

option(USE_STD_FILESYSTEM "Enable std::filesystem use for path and ImGuiFileDialog" OFF)
option(USE_NEW_IMGUI_KEY_SYSTEM "Define the symbol IMGUI_DISABLE_OBSOLETE_KEYIO" OFF)
option(USE_TEST "Enable Tests" OFF)

include(cmake/3rdparty.cmake)

if(MSVC)
    add_definitions(-DMSVC)
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wdeprecated-declarations -Wunused-parameter")
endif ()

add_definitions(${GLFW_DEFINITIONS})
add_definitions(-DCUSTOM_IMGUIFILEDIALOG_CONFIG="../../../CustomImGuiFileDialogConfig.h")
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if (USE_NEW_IMGUI_KEY_SYSTEM)
	add_definitions(-DIMGUI_DISABLE_OBSOLETE_KEYIO)
endif()

file(GLOB MAIN_SOURCES ${CMAKE_SOURCE_DIR}/main.cpp)
file(GLOB CUSTOM_SOURCES ${CMAKE_SOURCE_DIR}/CustomImGuiFileDialogConfig.h)
file(GLOB RES_SOURCES 
		${CMAKE_SOURCE_DIR}/CustomFont.cpp 
		${CMAKE_SOURCE_DIR}/CustomFont.h)
file(GLOB IMGUIFILEDIALOG_SOURCES 
		${CMAKE_SOURCE_DIR}/ImGuiFileDialog/*.cpp
		${CMAKE_SOURCE_DIR}/ImGuiFileDialog/*.h)
file(GLOB IMGUIIMPL_SOURCES 
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_glfw.cpp
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_glfw.h
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_opengl3.cpp
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_opengl3.h)
source_group(main FILES ${MAIN_SOURCES})
source_group(src\\Custom FILES ${CUSTOM_SOURCES})
source_group(src\\Res FILES ${RES_SOURCES})
source_group(src\\ImGuiFileDialog FILES ${IMGUIFILEDIALOG_SOURCES})
source_group(src\\ImGuiImpl FILES ${IMGUIIMPL_SOURCES})

if (APPLE)
	##set(ICON icon.icns)
	##set_source_files_properties(icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

add_library(${PROJECT_LIB} STATIC 
	${CUSTOM_SOURCES}
	${RES_SOURCES}
	${IMGUIFILEDIALOG_SOURCES}
	${IMGUIIMPL_SOURCES}
)

add_executable(${PROJECT_APP}
		MACOSX_BUNDLE
		##${ICON}
		${MAIN_SOURCES}
)

if (APPLE)
	set_target_properties(${PROJECT_APP}
			PROPERTIES
			OUTPUT_NAME "ImGuiFileDialog"
			MACOSX_BUNDLE_EXECUTABLE_NAME "ImGuiFileDialog"
			MACOSX_BUNDLE_BUNDLE_NAME "ImGuiFileDialog"
			FOLDER "ImGuiFileDialog"
			##RESOURCE icon.icns
			MACOSX_BUNDLE_SHORT_VERSION_STRING ""
			MACOSX_BUNDLE_LONG_VERSION_STRING ""
			##MACOSX_BUNDLE_ICON_FILE icon.icns
			MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/MacOSXBundleInfo.plist.in")
elseif(WIN32)
	set_target_properties(${PROJECT_APP} PROPERTIES OUTPUT_NAME "ImGuiFileDialog")
elseif(UNIX)
	set_target_properties(${PROJECT_APP} PROPERTIES OUTPUT_NAME "ImGuiFileDialog")
endif()

if (USE_STD_FILESYSTEM)
	add_definitions(-DUSE_STD_FILESYSTEM)
	set(CMAKE_CXX_STANDARD 17)
	set_target_properties(${PROJECT_APP} PROPERTIES CXX_STANDARD 17)
	set_target_properties(${PROJECT_LIB} PROPERTIES CXX_STANDARD 17)
	message("USE C++17 and std::filesystem")
else()
	set(CMAKE_CXX_STANDARD 11)
	set_target_properties(${PROJECT_APP} PROPERTIES CXX_STANDARD 11)
	set_target_properties(${PROJECT_LIB} PROPERTIES CXX_STANDARD 11)
	message("USE C++11 and DIRENT")
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}")
install(TARGETS ${PROJECT_APP} DESTINATION ${CMAKE_INSTALL_PREFIX})

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

target_link_libraries(${PROJECT_LIB} PRIVATE
	${OPENGL_LIBRARIES}
	${GLFW_LIBRARIES}
	${GLAD_LIBRARIES}
	${IMGUI_LIBRARIES}
)

set_target_properties(${PROJECT_LIB} PROPERTIES FOLDER Lib)

target_link_libraries(${PROJECT_APP} PRIVATE
	${PROJECT_LIB}
)

if(USE_TEST)
	enable_testing()
	add_definitions(-DNEED_TO_BE_PUBLIC_FOR_TESTS)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)
endif()
