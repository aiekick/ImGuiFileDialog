add_subdirectory(${CMAKE_SOURCE_DIR}/3rdparty/boost/)

set(BOOST_FILESYSTEM_INCLUDE_DIRS 
	${CMAKE_SOURCE_DIR}/3rdparty/boost/libs/filesystem/include
	${CMAKE_SOURCE_DIR}/3rdparty/boost/libs/assert/include
    ${CMAKE_SOURCE_DIR}/3rdparty/boost/libs/config/include
    ${CMAKE_SOURCE_DIR}/3rdparty/boost/libs/container_hash/include
    ${CMAKE_SOURCE_DIR}/3rdparty/boost/libs/core/include
    ${CMAKE_SOURCE_DIR}/3rdparty/boost/libs/detail/include
    ${CMAKE_SOURCE_DIR}/3rdparty/boost/libs/io/include
    ${CMAKE_SOURCE_DIR}/3rdparty/boost/libs/iterator/include
    ${CMAKE_SOURCE_DIR}/3rdparty/boost/libs/smart_ptr/include
    ${CMAKE_SOURCE_DIR}/3rdparty/boost/libs/system/include
    ${CMAKE_SOURCE_DIR}/3rdparty/boost/libs/type_traits/include
    ${CMAKE_SOURCE_DIR}/3rdparty/boost/libs/predef/include
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

## will serarhc for target with a particular prefix and will set destination folder
function(set_target_dir _PREFIX DIR TARGET_DIR)
    get_property(TGTS DIRECTORY "${DIR}" PROPERTY BUILDSYSTEM_TARGETS)
	string (REGEX MATCHALL "(^|;)${_PREFIX}[A-Za-z0-9_]*" _matchedVars "${TGTS}")
	foreach (_var IN LISTS _matchedVars)
		if (TARGET ${_var})
			set_target_properties(${_var} PROPERTIES FOLDER ${TARGET_DIR})
		endif()
	endforeach()
    get_property(SUBDIRS DIRECTORY "${DIR}" PROPERTY SUBDIRECTORIES)
    foreach(SUBDIR IN LISTS SUBDIRS)
        set_target_dir("${_PREFIX}" "${SUBDIR}" "${TARGET_DIR}")
    endforeach()
endfunction()

set_target_dir(boost . 3rdparty/boost)
