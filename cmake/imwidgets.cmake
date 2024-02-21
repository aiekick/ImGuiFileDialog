set(IMWIDGETS_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/imwidgets)
file(GLOB IMWIDGETS_SOURCES ${IMWIDGETS_INCLUDE_DIR}/*.cpp)
file(GLOB IMWIDGETS_HEADERS ${IMWIDGETS_INCLUDE_DIR}/*.h)
                 
add_library(imwidgets STATIC ${IMWIDGETS_SOURCES})

if (${CMAKE_SYSTEM_NAME} STREQUAL "Android")

add_definitions(-DIMGUI_IMPL_OPENGL_ES3)
include_directories(${IMWIDGETS_INCLUDE_DIR})

else()

add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)
include_directories(
    ${IMWIDGETS_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIR}
    ${GLAD_INCLUDE_DIR})    
target_link_libraries(imwidgets
    ${IMGUI_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${GLAD_LIBRARIES})
	
endif()
    
set_target_properties(imwidgets PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(imwidgets PROPERTIES FOLDER 3rdparty)

set(IMWIDGETS_LIBRARIES imwidgets)

