cmake_minimum_required (VERSION 3.5.1)

project(test_sharedmemory LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(test_sharedmemory
#    src/test_shared_memory.cpp
#    src/processes.cpp
    src/main.cpp)

include_directories(include)

#target_link_libraries(test_sharedmemory gtest)

#csp_add_unit_test(
#    TEST_SOURCES
#        test_shared_memory.cpp
#    INCLUDE_DIRS
#        ../public
#        ../src
#    LIBS
#        gtest
#        gmock
#        gtest_main
#        csp_afw_core_testing
#        csp_sfw_shared_memory
#        stubs_log_external
#    )
