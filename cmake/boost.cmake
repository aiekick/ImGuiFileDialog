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

set_target_properties(boost_assert PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_atomic PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_chrono PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_container PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_context PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_contract PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_core PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_coroutine PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_date_time PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_exception PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_fiber PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_fiber_numa PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_filesystem PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_graph PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_iostreams PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_json PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_locale PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_log PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_log_setup PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_nowide PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_prg_exec_monitor PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_program_options PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_random PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_serialization PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_stacktrace_basic PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_stacktrace_noop PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_stacktrace_windbg PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_stacktrace_windbg_cached PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_system PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_test_exec_monitor PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_thread PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_timer PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_type_erasure PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_unit_test_framework PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_url PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_variant2 PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_wave PROPERTIES FOLDER 3rdparty/boost)
set_target_properties(boost_wserialization PROPERTIES FOLDER 3rdparty/boost)