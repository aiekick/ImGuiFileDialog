cmake_minimum_required(VERSION 3.1)

set(PROJECT ImGuiFileDialog_Test)
enable_language(CXX)
project(${PROJECT} CXX)

file(GLOB_RECURSE PROJECT_TEST_SRC_RECURSE 
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp 
	${CMAKE_CURRENT_SOURCE_DIR}/*.h 
)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PROJECT_TEST_SRC_RECURSE})

add_executable(${PROJECT} 
	MACOSX_BUNDLE
	##${ICON}
	${PROJECT_TEST_SRC_RECURSE}
)
	
target_link_libraries(${PROJECT} PRIVATE
	${OPENGL_LIBRARIES}
	${GLFW_LIBRARIES}
	${GLAD_LIBRARIES}
	${IMGUI_LIBRARIES}
	ImGuiFileDialog
)

set_target_properties(${PROJECT} PROPERTIES FOLDER Tests)

#############################################################
#############################################################
#############################################################

include_directories("../src")

function(SetTest arg)
  add_test("${arg}" "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT}" "${arg}")
endfunction()

## IGFD::FilterManager -- Simple Filters
SetTest("IGFD::FilterManager::ParseFilters::Filters::Simple::0")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Simple::1")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Simple::2")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Simple::3")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Simple::4")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Simple::5")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Simple::6")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Simple::7")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Simple::8")

## IGFD::FilterManager -- Collection Filters
SetTest("IGFD::FilterManager::ParseFilters::Filters::Collection::0")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Collection::1")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Collection::2")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Collection::3")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Collection::4")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Collection::5")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Collection::6")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Collection::7")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Collection::8")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Collection::9")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Collection::10")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Collection::11")

## IGFD::FilterManager -- Simple Regex Filters
SetTest("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::0")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::1")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::2")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::3")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::4")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::5")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::6")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::7")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::8")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::9")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Simple::Regex::10")

## IGFD::FilterManager -- Collection Regex Filters
SetTest("IGFD::FilterManager::ParseFilters::Filters::Collection::Regex::0")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Collection::Regex::1")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Collection::Regex::2")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Collection::Regex::3")

## IGFD::FilterManager -- Divers Filters
SetTest("IGFD::FilterManager::ParseFilters::Filters::Divers::0")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Divers::1")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Divers::2")
SetTest("IGFD::FilterManager::ParseFilters::Filters::Divers::3")

## IGFD::FilterManager -- ReplaceExtentionWithCurrentFilter
SetTest("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::0")
SetTest("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::1")
SetTest("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::2")
SetTest("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::3")
SetTest("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::4")
SetTest("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::5")
SetTest("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::6")
SetTest("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::7")
SetTest("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::8")
SetTest("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::9")
SetTest("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::10")
SetTest("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::11")
SetTest("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::12")
SetTest("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::13")
SetTest("IGFD::FilterManager::ReplaceExtentionWithCurrentFilter::14")
