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

## Test_IGFD_FilterManager -- Simple Filters
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_0)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_1)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_2)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_3)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_4)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_5)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_6)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_7)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_8)

## Test_IGFD_FilterManager -- Collection Filters
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_0)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_1)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_2)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_3)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_4)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_5)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_6)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_7)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_8)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_9)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_10)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_11)

## Test_IGFD_FilterManager -- Simple Regex Filters
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_0)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_1)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_2)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_3)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_4)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_5)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_6)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_7)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_8)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_9)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_Simple_Regex_10)

## Test_IGFD_FilterManager -- Collection Regex Filters
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_Regex_0)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_Regex_1)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_Regex_2)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_Regex_3)

## Test_IGFD_FilterManager -- Divers Filters
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_Divers_0)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_Divers_1)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_Divers_2)
SetTest(Test_IGFD_FilterManager_ParseFilters_Filters_Divers_3)

## Test_IGFD_FilterManager -- ReplaceExtentionWithCurrentFilter
SetTest(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilter_0)
SetTest(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilter_1)
SetTest(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilter_2)
SetTest(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilter_3)
SetTest(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilter_4)
SetTest(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilter_5)
SetTest(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilter_6)
SetTest(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilter_7)
SetTest(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilter_8)
SetTest(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilter_9)
SetTest(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilter_10)
SetTest(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilter_11)
SetTest(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilter_12)
SetTest(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilter_13)
SetTest(Test_IGFD_FilterManager_ReplaceExtentionWithCurrentFilter_14)

## Test_IGFD_FilterManager -- FilterInfos -- transformAsteriskBasedFilterToRegex
SetTest(Test_IGFD_FilterManager_FilterInfos_transformAsteriskBasedFilterToRegex_0)

## Test_IGFD_FilterManager
SetTest(Test_IGFD_FilterManager_IsCoveredByFilters_0)
SetTest(Test_IGFD_FilterManager_IsCoveredByFilters_1)

## Test_IGFD_FileManager -- Filtering ASterisk
SetTest(Test_IGFD_FileManager_Filtering_asterisk_0)
SetTest(Test_IGFD_FileManager_Filtering_asterisk_1)
SetTest(Test_IGFD_FileManager_Filtering_asterisk_2)
SetTest(Test_IGFD_FileManager_Filtering_asterisk_3)

## Test_IGFD_FileManager -- Filtering Divers
SetTest(Test_IGFD_FileManager_Filtering_divers_0)
SetTest(Test_IGFD_FileManager_Filtering_divers_1)

## Test_IGFD_FileStyle
SetTest(Test_IGFD_FileStyle_ByExtention_0)
SetTest(Test_IGFD_FileStyle_ByExtention_1)
SetTest(Test_IGFD_FileStyle_ByExtention_2)
SetTest(Test_IGFD_FileStyle_ByExtention_3)
SetTest(Test_IGFD_FileStyle_ByExtention_4)
SetTest(Test_IGFD_FileStyle_ByExtention_5)