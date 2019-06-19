
#兼容Linux不同的架构
if (OPEN_LINUX_ARM)

elseif (x86)
    set (cbb_arch "-m32")
elseif (x64)
    set (cbb_arch "-m64")
endif(OPEN_LINUX_ARM)


if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${cbb_arch}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${cbb_arch}")
elseif (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	if (NOT IOS)
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${cbb_arch}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${cbb_arch}")
	endif ()
endif ()