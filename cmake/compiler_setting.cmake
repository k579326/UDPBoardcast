


# 设置DEBUG/Release标志
if (NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "RELEASE")
endif()
string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)


IF(CUSTOM_OS_WIN)
    # 忽略警告
    # LNK4099
    set(CMAKE_SHARED_LINKER_FLAGS   "${CMAKE_SHARED_LINKER_FLAGS}   /ignore:4099")
    set(CMAKE_EXE_LINKER_FLAGS      "${CMAKE_EXE_LINKER_FLAGS}   /ignore:4099")
    include (cmake/compiler_flags.cmake)              #vs的工程选项设置
	#set(CMAKE_GENERATOR_TOOLSET v140_xp)	    #设置vs平台工具集，兼容xp，但是这个好像不起作用，必须这样cmake .. -DCMAKE_GENERATOR_TOOLSET=v140_xp才有效
ELSE()
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
    #set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11") #为G++添加C++11支持
ENDIF()

IF(CUSTOM_OS_ANDROID)
    # android release 加 -s
    set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
   
    # android 测试需要
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pie -fPIE")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pie -fPIE")
    
ENDIF()

IF(CUSTOM_OS_MAC)
	set(CMAKE_OSX_DEPLOYMENT_TARGET 10.9)			# 最低支持mac10.9系统
#ELSEIF(CUSTOM_OS_IOS)
#	set(CMAKE_OSX_DEPLOYMENT_TARGET 8.0)			# 最低支持ios8.0
ENDIF()

IF (CUSTOM_OS_LINUX)
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}  -fPIC")
    set(CMAKE_CXX_FLAGS   "${CMAKE_CXX_FLAGS}  -fPIC")
    if (LINUX_ARCH STREQUAL "x86")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")
	    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
    elseif(LINUX_ARCH STREQUAL "x64")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m64")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
    endif()
ENDIF (CUSTOM_OS_LINUX)

















