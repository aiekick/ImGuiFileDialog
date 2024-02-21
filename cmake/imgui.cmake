set(IMGUI_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/imgui)
file(GLOB IMGUI_SOURCES ${IMGUI_INCLUDE_DIR}/*.cpp)
file(GLOB IMGUI_HEADERS ${IMGUI_INCLUDE_DIR}/*.h)
                 
add_library(imgui STATIC ${IMGUI_SOURCES})

add_definitions(-DIMGUI_DISABLE_OBSOLETE_FUNCTIONS)

if (${CMAKE_SYSTEM_NAME} STREQUAL "Android")

add_definitions(-DIMGUI_IMPL_OPENGL_ES3)
include_directories(${IMGUI_INCLUDE_DIR})

else()

add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)
include_directories(
    ${IMGUI_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIR}
    ${GLAD_INCLUDE_DIR})    
target_link_libraries(imgui
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${GLAD_LIBRARIES})
	
endif()
    
set_target_properties(imgui PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(imgui PROPERTIES FOLDER 3rdparty)

set(IMGUI_LIBRARIES imgui)

