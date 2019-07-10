MESSAGE(STATUS "[Operation System]: ${CMAKE_SYSTEM}:${CMAKE_SYSTEM_NAME}")
set (CUSTOM_OS_NAME "")
IF (CMAKE_SYSTEM_NAME STREQUAL "Windows")
	add_definitions(-DPF_OS_WIN)
	set (CUSTOM_OS_WIN ON)
	set (CUSTOM_OS_NAME "win")
ELSEIF (CMAKE_SYSTEM_NAME STREQUAL "Linux")
	add_definitions(-DPF_OS_LINUX)
	set (CUSTOM_OS_LINUX ON)
	set (CUSTOM_OS_NAME "linux")
ELSEIF (CMAKE_SYSTEM_NAME STREQUAL "Android")
	add_definitions(-DPF_OS_ANDROID)
	set (CUSTOM_OS_ANDROID ON)
	set (CUSTOM_OS_NAME "android")
ELSEIF (CMAKE_SYSTEM_NAME STREQUAL "Darwin")	# MAC OS X or IOS
	IF (NOT IOS)
		add_definitions(-DPF_OS_MAC)
		set (CUSTOM_OS_MAC ON)
		set (CUSTOM_OS_NAME "mac")
	ELSE()
		add_definitions(-DPF_OS_IOS)
		set (CUSTOM_OS_IOS ON)
		set (CUSTOM_OS_NAME "ios")
	ENDIF()
ENDIF()