option(linking_cmake_default "Build MD Target, default is MT" OFF)

#使用工程文件夹
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (NOT CRT_SHARED)
	set(CRT_SHARED "OFF")
endif()

string(TOUPPER ${CRT_SHARED} CRT_SHARED)

if(${CRT_SHARED} STREQUAL "OFF")
	message(STATUS "[CRT] MT")
elseif(${CRT_SHARED} STREQUAL "ON")
	message(FATAL_ERROR "[CRT] MD暂不支持！！！！！")
else()
	message(FATAL_ERROR "[CRT_SHARED]Invalid CRT build type.")
endif()

# MD改为MT, 要影响全局变量，所以只能使用macro
macro(fix_default_compiler_settings_)
	string(REGEX REPLACE "/W." "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	string(REGEX REPLACE "/W." "/W4" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    
	foreach(flag_var
        CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
		if(${CRT_SHARED} STREQUAL "OFF")
			string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")	
		elseif(${CRT_SHARED} STREQUAL "ON")
			string(REGEX REPLACE "/MT" "/MD" ${flag_var} "${${flag_var}}")
		endif()
		string(REGEX REPLACE "/O2" "/O1" ${flag_var} "${${flag_var}}")
        #[解决静态库依赖CRT的问题]设置cl编译器选项：忽略.obj 中的默认库
        # 解决静态库 Debug(MTD, MDD)和Release(MT, MD)混合链接时导致栈检查异常程序崩溃问题。
        #     此设置影响所有工程编译结果。此设置目的只针对静态库，故在下文对DLL和exe的链接选项进行修正。
		set(${flag_var} "${${flag_var}} /Zl")
   endforeach(flag_var)
   foreach(flag_var
        CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
		if(${CRT_SHARED} STREQUAL "OFF")
			string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
		elseif(${CRT_SHARED} STREQUAL "ON")
			string(REGEX REPLACE "/MT" "/MD" ${flag_var} "${${flag_var}}")
		endif()
		string(REGEX REPLACE "/O2" "/O1" ${flag_var} "${${flag_var}}")
        #[解决静态库依赖CRT的问题]设置cl编译器选项：忽略.obj 中的默认库
		set(${flag_var} "${${flag_var}} /Zl")
   endforeach(flag_var)
endmacro()

# 指定动态库依赖的CRT库
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG          "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}          LIBCMTD.lib LIBCPMTD.lib oldnames.lib")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE        "${CMAKE_SHARED_LINKER_FLAGS_RELEASE}        LIBCMT.lib LIBCPMT.lib oldnames.lib")
set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO} LIBCMT.lib LIBCPMT.lib oldnames.lib")
set(CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL     "${CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL}     LIBCMT.lib LIBCPMT.lib oldnames.lib")


# 指定exe依赖的CRT库
set(CMAKE_EXE_LINKER_FLAGS_DEBUG          "${CMAKE_EXE_LINKER_FLAGS_DEBUG}          LIBCMTD.lib LIBCPMTD.lib oldnames.lib")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE        "${CMAKE_EXE_LINKER_FLAGS_RELEASE}        LIBCMT.lib LIBCPMT.lib oldnames.lib")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} LIBCMT.lib LIBCPMT.lib oldnames.lib")
set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL     "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL}     LIBCMT.lib LIBCPMT.lib oldnames.lib")


macro(disable_warning)
    string(REGEX REPLACE "/W." "/w" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
	string(REGEX REPLACE "/W." "/w" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
endmacro()

macro(enable_warning)
	string(REGEX REPLACE "/w" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string(REGEX REPLACE "/w" "/W4" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
endmacro()

# 默认改为/MT编译
if(NOT linking_cmake_default)
	fix_default_compiler_settings_()
endif()

