

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


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include
					${CMAKE_CURRENT_SOURCE_DIR}/src
					)

set(UVSRC "")
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src UVSRC)
SET(UVSRC src/fs-poll.c
            src/inet.c
            src/threadpool.c
            src/uv-common.c
            src/version.c)

IF(CUSTOM_OS_WIN)
	set(LibDEP ws2_32 iphlpapi Psapi Userenv)
    aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/win UVSRC)
ELSE()
	set(LibDEP pthread)
	SET(UNIX_COMMON_SRC     src/unix/async.c
                        src/unix/atomic-ops.h
                        src/unix/core.c
                        src/unix/dl.c
                        src/unix/fs.c 
                        src/unix/getaddrinfo.c 
                        src/unix/getnameinfo.c 
                        src/unix/internal.h 
                        src/unix/loop-watcher.c 
                        src/unix/loop.c 
                        src/unix/pipe.c 
                        src/unix/poll.c 
                        src/unix/process.c 
                        src/unix/signal.c 
                        src/unix/spinlock.h 
                        src/unix/stream.c 
                        src/unix/tcp.c
                        src/unix/thread.c 
                        src/unix/timer.c 
                        src/unix/tty.c 
                        src/unix/udp.c
						)
		list(APPEND UVSRC ${UNIX_COMMON_SRC})
	IF (CUSTOM_OS_ANDROID)	
		set (ANDROID_SRC src/unix/android-ifaddrs.c 
                         src/unix/pthread-fixes.c 
                         src/unix/pthread-barrier.c
                         
                         src/unix/linux-core.c
                         src/unix/linux-inotify.c
                         src/unix/linux-syscalls.c
                         src/unix/proctitle.c
                        )
		list(APPEND UVSRC ${ANDROID_SRC})
	ELSEIF (CUSTOM_OS_MAC OR CUSTOM_OS_IOS)
		set (MAC_SRC 	src/unix/proctitle.c
				src/unix/darwin.c
				src/unix/fsevents.c
				src/unix/darwin-proctitle.c
				src/unix/pthread-barrier.c
				src/unix/kqueue.c)
		list(APPEND UVSRC ${MAC_SRC})
		
		add_definitions(-D_DARWIN_USE_64_BIT_INODE=1)
		add_definitions(-D_DARWIN_UNLIMITED_SELECT=1)
		
	ELSEIF (CUSTOM_OS_LINUX)
		set (LINUX_SRC src/unix/linux-core.c
                    src/unix/linux-inotify.c
                    src/unix/linux-syscalls.c
                    src/unix/linux-syscalls.h
                    src/unix/proctitle.c)
		list(APPEND UVSRC ${LINUX_SRC})		
		add_definitions(-D_GNU_SOURCE)
					
	ELSE()
		message(FATAL_ERROR "libuv don't support target OS!")
	ENDIF()
ENDIF()

add_library(libuv STATIC ${UVSRC})
target_link_libraries(libuv ${LibDEP})
set_target_properties(libuv PROPERTIES FOLDER "third")
