FetchContent_Declare(
	boost
	GIT_REPOSITORY	https://github.com/boostorg/boost.git
	GIT_TAG			boost-1.86.0
	SOURCE_DIR		${CMAKE_CURRENT_SOURCE_DIR}/build/_deps/boost
	GIT_PROGRESS	true
	GIT_SHALLOW		true
)

FetchContent_GetProperties(boost)
if(NOT boost_POPULATED)
	FetchContent_Populate(boost)
	
	##EXCLUDE_FROM_ALL reject install for this target
	add_subdirectory(${boost_SOURCE_DIR} EXCLUDE_FROM_ALL)

	set(BOOST_FILESYSTEM_INCLUDE_DIRS 
		${boost_SOURCE_DIR}/libs/filesystem/include
		${boost_SOURCE_DIR}/libs/assert/include
		${boost_SOURCE_DIR}/libs/config/include
		${boost_SOURCE_DIR}/libs/container_hash/include
		${boost_SOURCE_DIR}/libs/core/include
		${boost_SOURCE_DIR}/libs/detail/include
		${boost_SOURCE_DIR}/libs/io/include
		${boost_SOURCE_DIR}/libs/iterator/include
		${boost_SOURCE_DIR}/libs/smart_ptr/include
		${boost_SOURCE_DIR}/libs/system/include
		${boost_SOURCE_DIR}/libs/type_traits/include
		${boost_SOURCE_DIR}/libs/predef/include
	)

	set(BOOST_FILESYSTEM_LIBRARIES
		boost_assert
		boost_config
		boost_container_hash
		boost_core
		boost_detail
		boost_filesystem    
		boost_io
		boost_iterator
		boost_predef
		boost_smart_ptr
		boost_system
		boost_type_traits
	)

	## will search for target with a particular prefix and will set destination folder
	function(set_target_dir _PREFIX DIR TARGET_DIR)
		get_property(TGTS DIRECTORY "${DIR}" PROPERTY BUILDSYSTEM_TARGETS)
		string (REGEX MATCHALL "(^|;)${_PREFIX}[A-Za-z0-9_]*" _matchedVars "${TGTS}")
		foreach (_var IN LISTS _matchedVars)
			if (TARGET ${_var})
				set_target_properties(${_var} PROPERTIES FOLDER ${TARGET_DIR})
				if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
				else()
					target_compile_options(${_var} INTERFACE "-Wno-everything") 
				endif()
			endif()
		endforeach()
		get_property(SUBDIRS DIRECTORY "${DIR}" PROPERTY SUBDIRECTORIES)
		foreach(SUBDIR IN LISTS SUBDIRS)
			set_target_dir("${_PREFIX}" "${SUBDIR}" "${TARGET_DIR}")
		endforeach()
	endfunction()

	set_target_dir(boost . ${boost_SOURCE_DIR}/libs)
endif()

