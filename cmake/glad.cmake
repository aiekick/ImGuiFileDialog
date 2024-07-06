set(GLAD_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/3rdparty/glad/include)
file(GLOB GLAD_SOURCES ${CMAKE_SOURCE_DIR}/3rdparty/glad/src/glad.c)
file(GLOB GLAD_HEADERS ${GLAD_INCLUDE_DIR}/glad/glad.h)

add_library(glad STATIC ${GLAD_HEADERS} ${GLAD_SOURCES})

include_directories(${GLAD_INCLUDE_DIR})

set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)
set_target_properties(glad PROPERTIES FOLDER 3rdparty)
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    #target_compile_options(glad PRIVATE "-Wno-everything") 
else()
    target_compile_options(glad PRIVATE "-Wno-everything") 
endif()

set(GLAD_LIBRARIES glad)

if(NOT WIN32)
    set(GLAD_LIBRARIES ${GLAD_LIBRARIES} dl)
endif()
