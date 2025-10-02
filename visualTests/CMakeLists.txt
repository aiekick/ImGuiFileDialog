cmake_minimum_required(VERSION 3.20)

set(PROJECT ImGuiFileDialog_VisualTests)
project(${PROJECT} CXX)
enable_language(C CXX)
set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SRC_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.*)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX src FILES ${SRC_SOURCES})

file(GLOB MAIN_SOURCES 
	${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
source_group(src/main FILES ${MAIN_SOURCES})

file(GLOB IMGUI_SOURCES 
	${CMAKE_SOURCE_DIR}/3rdparty/imgui/*.cpp
	${CMAKE_SOURCE_DIR}/3rdparty/imgui/*.h)
source_group(src/ImGui FILES ${IMGUI_SOURCES})

file(GLOB INGUI_TEST_ENGINE_SOURCES 
	${CMAKE_SOURCE_DIR}/3rdparty/imgui_test_engine/imgui_test_engine/*.h
	${CMAKE_SOURCE_DIR}/3rdparty/imgui_test_engine/imgui_test_engine/*.cpp)
source_group(src/ImGuiTestEngine/Engine FILES ${INGUI_TEST_ENGINE_SOURCES})

file(GLOB IMGUI_TEST_ENGINE_APP_SOURCES 
	${CMAKE_SOURCE_DIR}/3rdparty/imgui_test_engine/shared/imgui_app.cpp
	${CMAKE_SOURCE_DIR}/3rdparty/imgui_test_engine/shared/imgui_app.h)
source_group(src/ImGuiTestEngine/App FILES ${IMGUI_TEST_ENGINE_APP_SOURCES})

file(GLOB IGFD_SOURCES 
	${CMAKE_SOURCE_DIR}/ImGuiFileDialog/*.cpp
	${CMAKE_SOURCE_DIR}/ImGuiFileDialog/*.h)
source_group(src/ImGuiFileDialog FILES ${IGFD_SOURCES})

add_executable(${PROJECT} 
	${SRC_SOURCES}
	${MAIN_SOURCES}
	${IGFD_SOURCES}
	${IMGUI_SOURCES}
	${INGUI_TEST_ENGINE_SOURCES}
	${IMGUI_TEST_ENGINE_APP_SOURCES}
)

if (LINUX)
	target_link_libraries(${PROJECT} PRIVATE X11)
endif()

target_link_libraries(${PROJECT} PRIVATE
	${GLAD_LIBRARIES}
	${GLFW_LIBRARIES}
	${OPENGL_LIBRARIES}
)

target_include_directories(${PROJECT} PRIVATE 
	${CMAKE_CURRENT_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/ImGuiFileDialog
	${CMAKE_SOURCE_DIR}/3rdparty/imgui/backends
	${CMAKE_SOURCE_DIR}/3rdparty/imgui
	${CMAKE_SOURCE_DIR}/3rdparty/imgui_test_engine
)

target_compile_definitions(${PROJECT} PRIVATE 
	IMGUI_DEFINE_MATH_OPERATORS
	IMGUI_APP_GLFW_GL3
	IMGUI_ENABLE_TEST_ENGINE
	IMGUI_IMPL_OPENGL_LOADER_GLAD
	IMGUI_TEST_ENGINE_ENABLE_CAPTURE
	IMGUI_TEST_ENGINE_ENABLE_COROUTINE_STDTHREAD_IMPL
	CUSTOM_IMGUIFILEDIALOG_CONFIG="${CMAKE_CURRENT_SOURCE_DIR}/src/headers/customIgfdConfig.h"
	SAMPLES_DIRECTORY="${CMAKE_SOURCE_DIR}/samples"
)

set_target_properties(${PROJECT} PROPERTIES CXX_STANDARD 17)

set_target_properties(${PROJECT} PROPERTIES FOLDER Tests/ImGuiTestEngine)

set_target_properties(${PROJECT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${FINAL_BIN_DIR}")
set_target_properties(${PROJECT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${FINAL_BIN_DIR}")
set_target_properties(${PROJECT} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${FINAL_BIN_DIR}")
