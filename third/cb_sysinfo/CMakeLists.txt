project(cb_sysinfo)
cmake_minimum_required(VERSION 2.8)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)


if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
set(SRCDIR win)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
set(SRCDIR linux)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Android")
set(SRCDIR android)
elseif (CMAKE_SYSTEM_NAME STREQUAL "darwin")
    if (NOT DEFINED IOS)
        set(SRCDIR ios)
    else()
        set(SRCDIR mac)
    endif()
endif()
    
if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set (DEPENDLIBS version)
elseif (CMAKE_SYSTEM_NAME STREQUAL "darwin")
    set (DEPENDLIBS "-framework CoreServices")
endif()

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/${SRCDIR} SRCFILE)
add_library(cb_sysinfo STATIC ${SRCFILE})
target_link_libraries(cb_sysinfo ${DEPENDLIBS})

set_target_properties(cb_sysinfo PROPERTIES FOLDER "third")



