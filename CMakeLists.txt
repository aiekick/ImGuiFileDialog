cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0048 NEW)
cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0012 NEW)
cmake_policy(SET CMP0023 NEW)

set(PROJECT ImGuiFileDialog)

enable_language(C CXX)
project(${PROJECT} CXX)

include(cmake/3rdparty.cmake)

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

add_definitions(${GLFW_DEFINITIONS})
add_definitions(-DCUSTOM_IMGUIFILEDIALOG_CONFIG="../../../CustomImGuiFileDialogConfig.h")
   
file(GLOB MAIN_SOURCES ${CMAKE_SOURCE_DIR}/main.cpp)
file(GLOB CUSTOM_SOURCES ${CMAKE_SOURCE_DIR}/CustomImGuiFileDialogConfig.h)
file(GLOB RES_SOURCES 
		${CMAKE_SOURCE_DIR}/CustomFont.cpp 
		${CMAKE_SOURCE_DIR}/CustomFont.h)
file(GLOB IMGUIFILEDIALOG_SOURCES 
		${CMAKE_SOURCE_DIR}/ImGuiFileDialog/*.cpp
		${CMAKE_SOURCE_DIR}/ImGuiFileDialog/*.h)
file(GLOB IMGUIIMPL_SOURCES 
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/examples/imgui_impl_glfw.cpp
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/examples/imgui_impl_glfw.h
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/examples/imgui_impl_opengl3.cpp
		${CMAKE_SOURCE_DIR}/3rdparty/imgui/examples/imgui_impl_opengl3.h)
source_group(main FILES ${MAIN_SOURCES})
source_group(src\\Custom FILES ${CUSTOM_SOURCES})
source_group(src\\Res FILES ${RES_SOURCES})
source_group(src\\ImGuiFileDialog FILES ${IMGUIFILEDIALOG_SOURCES})
source_group(src\\ImGuiImpl FILES ${IMGUIIMPL_SOURCES})

if (APPLE)
	##set(ICON icon.icns)
	##set_source_files_properties(icon.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
endif()

add_executable(${PROJECT}
		MACOSX_BUNDLE
		##${ICON}
		${MAIN_SOURCES}
		${CUSTOM_SOURCES}
		${RES_SOURCES}
		${IMGUIFILEDIALOG_SOURCES}
		${IMGUIIMPL_SOURCES}
)

if (APPLE)
	set_target_properties(${PROJECT}
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
	set_target_properties(${PROJECT} PROPERTIES OUTPUT_NAME "ImGuiFileDialog")
elseif(UNIX)
	set_target_properties(${PROJECT} PROPERTIES OUTPUT_NAME "ImGuiFileDialog")
endif()
set_target_properties(${PROJECT} PROPERTIES CXX_STANDARD 11)

set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}")
install(TARGETS ${PROJECT} DESTINATION ${CMAKE_INSTALL_PREFIX})

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

target_link_libraries(${PROJECT} PRIVATE
	${OPENGL_LIBRARIES}
        ${GLFW_LIBRARIES}
        ${GLAD_LIBRARIES}
        ${IMGUI_LIBRARIES}
)

