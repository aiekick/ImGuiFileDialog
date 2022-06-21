cmake_minimum_required(VERSION 3.1)

set(PROJECT ImGuiFileDialog_Test)
enable_language(C CXX)
project(${PROJECT} CXX)

file(GLOB_RECURSE PROJECT_TEST_SRC_RECURSE 
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/*.h 
)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PROJECT_TEST_SRC_RECURSE})

add_executable(${PROJECT} ${PROJECT_TEST_SRC_RECURSE})

target_link_libraries(${PROJECT} PRIVATE 
	ImGuiFileDialog_LIB 
)

set_target_properties(${PROJECT} PROPERTIES FOLDER Tests)

#############################################################
#############################################################
#############################################################

include_directories("../src")

function(SetTest arg)
  add_test("${arg}" "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT}" "${arg}")
endfunction()

## IGFD::FilterManager -- Collection Filters
SetTest("IGFD::FilterManager::ParseFilters::CollectionFilters::0")
SetTest("IGFD::FilterManager::ParseFilters::CollectionFilters::1")
SetTest("IGFD::FilterManager::ParseFilters::CollectionFilters::2")
