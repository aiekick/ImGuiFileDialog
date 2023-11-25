cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0079 NEW) # for line 67

set(PROJECT ImGuiFileDialog_App)

enable_language(C CXX)
project(${PROJECT} CXX)

option(USE_NEW_IMGUI_KEY_SYSTEM "Define the symbol IMGUI_DISABLE_OBSOLETE_KEYIO" ON)
option(USE_TEST "Enable tests" ON)

option(USE_STD_FILESYSTEM "Enable std::filesystem use for path and ImGuiFileDialog" OFF)
option(USE_BOOST_DEMO_FILESYSTEM "Enable the demo of custom filesystem here with boost" OFF)

include(cmake/3rdparty.cmake)

if (USE_BOOST_DEMO_FILESYSTEM)
	add_definitions(-DCUSTOM_FILESYSTEM_INCLUDE="${CMAKE_SOURCE_DIR}/FileSystemBoost.hpp")
endif()

if(MSVC)
    add_definitions(-DMSVC)
	if (USE_STD_FILESYSTEM)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++11")
	endif()
else ()
	if (USE_STD_FILESYSTEM)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra -Wpedantic -Wdeprecated-declarations -Wunused-parameter")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -Wpedantic -Wdeprecated-declarations -Wunused-parameter")
	endif()
endif ()

add_definitions(${GLFW_DEFINITIONS})
add_definitions(-DIMGUI_DISABLE_OBSOLETE_FUNCTIONS)
add_definitions(-DCUSTOM_IMGUIFILEDIALOG_CONFIG="${CMAKE_SOURCE_DIR}/CustomImGuiFileDialogConfig.h")
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if (USE_NEW_IMGUI_KEY_SYSTEM)
	add_definitions(-DIMGUI_DISABLE_OBSOLETE_KEYIO)
endif()

file(GLOB MAIN_SOURCES ${CMAKE_SOURCE_DIR}/main.cpp)
file(GLOB CUSTOM_SOURCES ${CMAKE_SOURCE_DIR}/CustomImGuiFileDialogConfig.h)

if (USE_BOOST_DEMO_FILESYSTEM)
	file(GLOB BOOST_SOURCES ${CMAKE_SOURCE_DIR}/FileSystemBoost.hpp)
	source_group(src\\Boost FILES ${BOOST_SOURCES})
endif()

file(GLOB RES_SOURCES 
		${CMAKE_SOURCE_DIR}/CustomFont.cpp 
		${CMAKE_SOURCE_DIR}/CustomFont.h)
file(GLOB IMGUIIMPL_SOURCES 
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_glfw.cpp
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_glfw.h
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_opengl3.cpp
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_opengl3.h)
source_group(main FILES ${MAIN_SOURCES})
source_group(src\\Custom FILES ${CUSTOM_SOURCES})
source_group(src\\Res FILES ${RES_SOURCES})
source_group(src\\ImGuiImpl FILES ${IMGUIIMPL_SOURCES})

if (APPLE)
	##set(ICON icon.icns)
	##set_source_files_properties(icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

include_directories(
	${GLM_INCLUDE_DIR}
	${PLOG_INCLUDE_DIR}
	${GLFW_INCLUDE_DIR}
	${GLAD_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}
	${IMGUI_INCLUDE_DIR}
	${OPENGL_INCLUDE_DIR}
	${BOOST_FILESYSTEM_INCLUDE_DIRS}
)

if (USE_TEST)
	add_definitions(-DNEED_TO_BE_PUBLIC_FOR_TESTS)
endif()

add_subdirectory(${CMAKE_SOURCE_DIR}/ImGuiFileDialog)

target_link_libraries(ImGuiFileDialog PRIVATE
	${IMGUI_LIBRARIES}
	${BOOST_FILESYSTEM_LIBRARIES}
)

add_executable(${PROJECT}
	MACOSX_BUNDLE
	##${ICON}
	${RES_SOURCES}
	${MAIN_SOURCES}
	${BOOST_SOURCES}
	${CUSTOM_SOURCES}
	${IMGUIIMPL_SOURCES}
)

if(UNIX)
    target_compile_options(${PROJECT} PUBLIC "-Wno-unknown-pragmas")
endif()

if (APPLE)
	set_target_properties(${PROJECT}
		PROPERTIES
		OUTPUT_NAME "${PROJECT}_Macos"
		MACOSX_BUNDLE_EXECUTABLE_NAME "${PROJECT}"
		MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT}"
		FOLDER "${PROJECT}"
		##RESOURCE icon.icns
		MACOSX_BUNDLE_SHORT_VERSION_STRING ""
		MACOSX_BUNDLE_LONG_VERSION_STRING ""
		##MACOSX_BUNDLE_ICON_FILE icon.icns
		MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/MacOSXBundleInfo.plist.in")
elseif(WIN32)
	set_target_properties(${PROJECT} PROPERTIES OUTPUT_NAME "${PROJECT}_WIn32")
elseif(UNIX)
	set_target_properties(${PROJECT} PROPERTIES OUTPUT_NAME "${PROJECT}_Unix")
endif()

if (USE_STD_FILESYSTEM)
	add_definitions(-DUSE_STD_FILESYSTEM)
	set(CMAKE_CXX_STANDARD 17)
	set_target_properties(${PROJECT} PROPERTIES CXX_STANDARD 17)
	set_target_properties(ImGuiFileDialog PROPERTIES CXX_STANDARD 17)
	set_target_properties(imgui PROPERTIES CXX_STANDARD 17)
	message("USE C++17 and std::filesystem")
else()
	set(CMAKE_CXX_STANDARD 11)
	set_target_properties(${PROJECT} PROPERTIES CXX_STANDARD 11)
	set_target_properties(ImGuiFileDialog PROPERTIES CXX_STANDARD 11)
	set_target_properties(imgui PROPERTIES CXX_STANDARD 11)
	if (USE_BOOST_DEMO_FILESYSTEM)
		message("USE C++11 and BOOST as demo custom filesystem")
	else()
		message("USE C++11 and DIRENT")
	endif()
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}")
install(TARGETS ${PROJECT} DESTINATION ${CMAKE_INSTALL_PREFIX})

target_link_libraries(${PROJECT} PRIVATE
	${OPENGL_LIBRARIES}
	${GLFW_LIBRARIES}
	${GLAD_LIBRARIES}
	${IMGUI_LIBRARIES}
	ImGuiFileDialog
)

set_target_properties(ImGuiFileDialog PROPERTIES FOLDER Lib)

target_include_directories(ImGuiFileDialog PRIVATE
	${CMAKE_SOURCE_DIR}
)

if (USE_TEST)
	enable_testing()
	add_subdirectory(${CMAKE_SOURCE_DIR}/tests)
endif()