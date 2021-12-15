cmake_minimum_required (VERSION 3.5.1)

project(test_sharedmemory LANGUAGES C)

add_executable(test_sharedmemory
    main.c)

if(CMAKE_SYSTEM_NAME STREQUAL "QNX")
else()
	target_link_libraries(test_sharedmemory rt pthread)
endif()

