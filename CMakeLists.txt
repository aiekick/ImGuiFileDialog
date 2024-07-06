cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0079 NEW) # for line 67

set(PROJECT ImGuiFileDialog_App)

enable_language(C CXX)
project(${PROJECT} CXX)

option(USE_NEW_IMGUI_KEY_SYSTEM "Define the symbol IMGUI_DISABLE_OBSOLETE_KEYIO" ON)
option(USE_TEST "Enable tests" ON)

option(USE_DIRENT_FILESYSTEM "Enable the default filesystem with dirent" ON)
option(USE_STD_FILESYSTEM "Enable std::filesystem use for path and ImGuiFileDialog" OFF)
option(USE_BOOST_FILESYSTEM "Enable the demo of custom filesystem here with boost" OFF)

## some defines for debug mode (before 3rdparty.cmake)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

## some defines for debug mode (before 3rdparty.cmake)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

set(FINAL_BIN_DIR ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_PDB_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}/debug")

set(CMAKE_INSTALL_PREFIX .)
set(CMAKE_INSTALL_BINDIR .)

if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX _debug)
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Android")
	set(USE_STD_FILESYSTEM ON CACHE BOOL "" FORCE)
endif()

if (UNIX)
	option(USE_LEAK_SANITIZER "Enable the Leak Sanitizer" OFF)
endif()

include(cmake/3rdparty.cmake)

if (USE_BOOST_FILESYSTEM)
	add_definitions(-DCUSTOM_FILESYSTEM_INCLUDE="${CMAKE_CURRENT_SOURCE_DIR}/FileSystemBoost.hpp")
endif()

set(ORIGINAL_CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_definitions(-DMSVC)
	if (USE_STD_FILESYSTEM)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++11")
	endif()
else()
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Wdeprecated-declarations -Wunused-parameter")
	if (USE_STD_FILESYSTEM)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	endif()
endif()

add_definitions(${GLFW_DEFINITIONS})
add_definitions(-DIMGUI_DISABLE_OBSOLETE_FUNCTIONS)
add_definitions(-DCUSTOM_IMGUIFILEDIALOG_CONFIG="${CMAKE_CURRENT_SOURCE_DIR}/CustomImGuiFileDialogConfig.h")
add_definitions(-DCUSTOM_IMWIDGETS_CONFIG="${CMAKE_CURRENT_SOURCE_DIR}/CustomImWidgetsConfig.h")
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

if (USE_NEW_IMGUI_KEY_SYSTEM)
	add_definitions(-DIMGUI_DISABLE_OBSOLETE_KEYIO)
endif()

file(GLOB CUSTOM_SOURCES 
	${CMAKE_CURRENT_SOURCE_DIR}/CustomImGuiFileDialogConfig.h
	${CMAKE_CURRENT_SOURCE_DIR}/CustomImWidgetsConfig.h)

if (USE_BOOST_FILESYSTEM)
	file(GLOB BOOST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/FileSystemBoost.hpp)
	source_group(src\\Boost FILES ${BOOST_SOURCES})
elseif (USE_STD_FILESYSTEM)
	add_definitions(-DUSE_STD_FILESYSTEM)
endif()

file(GLOB RES_SOURCES 
		${CMAKE_CURRENT_SOURCE_DIR}/CustomFont.cpp 
		${CMAKE_CURRENT_SOURCE_DIR}/CustomFont.h
		${CMAKE_CURRENT_SOURCE_DIR}/Roboto_Medium.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/Roboto_Medium.h)
file(GLOB RES_WIDGETS 
		${CMAKE_CURRENT_SOURCE_DIR}/ImWidgets.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/ImWidgets.h)
file(GLOB RES_DEMO_DIALOG
		${CMAKE_CURRENT_SOURCE_DIR}/DemoDialog.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/DemoDialog.h)

file(GLOB_RECURSE CMAKE_SOURCES ${CMAKE_SOURCE_DIR}/cmake/*.*)
		
if (${CMAKE_SYSTEM_NAME} STREQUAL "Android") 
	file(GLOB MAIN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/mainAndroid.cpp)
	file(GLOB IMGUIIMPL_SOURCES 
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_android.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_android.h
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_opengl3.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_opengl3.h
		${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)
else()
	file(GLOB MAIN_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/mainWinLinuxMacos.cpp)
	file(GLOB IMGUIIMPL_SOURCES 
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_glfw.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_glfw.h
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_opengl3.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/imgui/backends/imgui_impl_opengl3.h)
endif()

source_group(cmake FILES ${CMAKE_SOURCES})
source_group(src\\Res FILES ${RES_SOURCES})
source_group(src\\Widgets FILES ${RES_WIDGETS})
source_group(src\\Custom FILES ${CUSTOM_SOURCES})
source_group(src\\DemoDialog FILES ${RES_DEMO_DIALOG})
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
	${CMAKE_CURRENT_SOURCE_DIR}
	${IMGUI_INCLUDE_DIR}
	${IMWIDGETS_INCLUDE_DIR}
	${OPENGL_INCLUDE_DIR}
	${BOOST_FILESYSTEM_INCLUDE_DIRS}
)

if (USE_TEST)
	add_definitions(-DNEED_TO_BE_PUBLIC_FOR_TESTS)
endif()

if (USE_LEAK_SANITIZER)
	add_compile_options(-fexceptions -pthread -ldl -lGL -Wall -g -fsanitize=undefined -fno-sanitize-recover -fsanitize=float-cast-overflow -fsanitize=leak -fsanitize=address -fsanitize-address-use-after-scope -fstack-protector -fstack-protector-all)
	add_link_options(-fexceptions -pthread -ldl -lGL -Wall -g -fsanitize=undefined -fno-sanitize-recover -fsanitize=float-cast-overflow -fsanitize=leak -fsanitize=address -fsanitize-address-use-after-scope -fstack-protector -fstack-protector-all)
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ImGuiFileDialog)

target_link_libraries(ImGuiFileDialog PRIVATE
	${IMGUI_LIBRARIES}
	${IMWIDGETS_LIBRARIES}
	${BOOST_FILESYSTEM_LIBRARIES}
)

set(SOURCES 
	${RES_SOURCES}
	${RES_WIDGETS}
	${MAIN_SOURCES}
	${BOOST_SOURCES}
	${CMAKE_SOURCES}
	${CUSTOM_SOURCES}
	${RES_DEMO_DIALOG}
	${IMGUIIMPL_SOURCES})

if (${CMAKE_SYSTEM_NAME} STREQUAL "Android") 
	add_library(${PROJECT} SHARED ${SOURCES})
else()
	add_executable(${PROJECT} MACOSX_BUNDLE ${SOURCES})
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}")
install(TARGETS ${PROJECT} DESTINATION ${CMAKE_INSTALL_PREFIX})

if(UNIX)
    target_compile_options(${PROJECT} PUBLIC -Wno-unknown-pragmas)
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
		MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/MacOSXBundleInfo.plist.in")
elseif(WIN32)
	set_target_properties(${PROJECT} PROPERTIES OUTPUT_NAME "${PROJECT}_Win32")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Android")

elseif(UNIX)
	set_target_properties(${PROJECT} PROPERTIES OUTPUT_NAME "${PROJECT}_Unix")
endif()

if (USE_STD_FILESYSTEM)
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
	if (USE_BOOST_FILESYSTEM)
		message("USE C++11 and BOOST as demo custom filesystem")
	else()
		message("USE C++11 and DIRENT")
	endif()
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}")
install(TARGETS ${PROJECT} DESTINATION ${CMAKE_INSTALL_PREFIX})

if(${CMAKE_SYSTEM_NAME} STREQUAL "Android")
target_link_libraries(${PROJECT} PRIVATE
	${IMGUI_LIBRARIES}
	${IMWIDGETS_LIBRARIES}
	${BOOST_FILESYSTEM_LIBRARIES}
	ImGuiFileDialog
	android
	EGL
	GLESv3
	log)
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../..
  ${ANDROID_NDK}/sources/android/native_app_glue)
else()
target_link_libraries(${PROJECT} PRIVATE
	${OPENGL_LIBRARIES}
	${GLFW_LIBRARIES}
	${GLAD_LIBRARIES}
	${IMGUI_LIBRARIES}
	${IMWIDGETS_LIBRARIES}
	ImGuiFileDialog)
endif()

set_target_properties(ImGuiFileDialog PROPERTIES FOLDER Lib)

target_include_directories(ImGuiFileDialog PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
)

if (USE_TEST)
	enable_testing()
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)
endif()

set_target_properties(${PROJECT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}")
set_target_properties(${PROJECT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${FINAL_BIN_DIR}")
set_target_properties(${PROJECT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${FINAL_BIN_DIR}")
