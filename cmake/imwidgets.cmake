set(IMWIDGETS_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/imwidgets)
file(GLOB IMWIDGETS_SOURCES ${IMWIDGETS_INCLUDE_DIR}/*.cpp)
file(GLOB IMWIDGETS_HEADERS ${IMWIDGETS_INCLUDE_DIR}/*.h)
                 
add_library(imwidgets STATIC ${IMWIDGETS_SOURCES})

include_directories(
    ${IMWIDGETS_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIR}
    ${GLAD_INCLUDE_DIR})
    
target_link_libraries(imwidgets
    ${OPENGL_LIBRARIES}
	${IMGUI_LIBRARIES}
    ${GLFW_LIBRARIES}
    ${GLAD_LIBRARIES})
    
set_target_properties(imwidgets PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(imwidgets PROPERTIES FOLDER 3rdparty)

set(IMWIDGETS_LIBRARIES imwidgets)

