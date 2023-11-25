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

set_target_properties(boost PROPERTIES FOLDER 3rdparty/boost)
