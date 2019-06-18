project(cb_thread)
cmake_minimum_required(VERSION 2.8)

include(cmake/select_arch.cmake)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	set(SS_PLATFORM src/win)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
	set(SS_PLATFORM src/android)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(SS_PLATFORM src/linux)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin") #Mac OS X or IOS
    if (NOT IOS)
        set(SS_PLATFORM src/mac)
    else()
        set(SS_PLATFORM src/ios)
    endif()
endif()

if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/gtest-1.7.0/CMakeLists.txt)
    add_subdirectory(gtest-1.7.0)
endif()

if (gtest_SOURCE_DIR)
    include(cmake/compiler_flags.cmake)
    set_crt_mt()
endif()
	
AUX_SOURCE_DIRECTORY(${SS_PLATFORM} SS_LIB_SRC)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)
add_library(cb_thread STATIC ${SS_LIB_SRC})

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
target_link_libraries(cb_thread pthread rt)
endif()

set_target_properties(cb_thread PROPERTIES "FOLDER" "third")

if (gtest_SOURCE_DIR)
    #include_directories(${gtest_SOURCE_DIR}/include)
    #add_subdirectory(test)
endif()
