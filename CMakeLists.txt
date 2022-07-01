cmake_minimum_required(VERSION 3.6)

project(workflow_starter)

# --- compile workflow --- #
# macro definition
if (WIN32)
	set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   /MP /wd4200")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /wd4200 /Zc:__cplusplus /std:c++14")
else ()
	set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wall -fPIC -pipe -std=gnu90")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC -pipe -std=c++11 -fno-exceptions")
endif ()

# add header path
if (WIN32)
    include_directories(
    	${CMAKE_CURRENT_SOURCE_DIR}/workflow-0.9.6/workflow-windows/src
        ${CMAKE_CURRENT_SOURCE_DIR}/workflow-0.9.6/workflow-windows/src/algorithm
        ${CMAKE_CURRENT_SOURCE_DIR}/workflow-0.9.6/workflow-windows/src/client
        ${CMAKE_CURRENT_SOURCE_DIR}/workflow-0.9.6/workflow-windows/src/factory
        ${CMAKE_CURRENT_SOURCE_DIR}/workflow-0.9.6/workflow-windows/src/kernel_win
        ${CMAKE_CURRENT_SOURCE_DIR}/workflow-0.9.6/workflow-windows/src/manager
        ${CMAKE_CURRENT_SOURCE_DIR}/workflow-0.9.6/workflow-windows/src/nameservice
        ${CMAKE_CURRENT_SOURCE_DIR}/workflow-0.9.6/workflow-windows/src/protocol
        ${CMAKE_CURRENT_SOURCE_DIR}/workflow-0.9.6/workflow-windows/src/server
        ${CMAKE_CURRENT_SOURCE_DIR}/workflow-0.9.6/workflow-windows/src/util
    )
elseif (UNIX)
    include_directories(
 	${CMAKE_CURRENT_SOURCE_DIR}/workflow-0.9.6/workflow-linux/src
        ${CMAKE_CURRENT_SOURCE_DIR}/workflow-0.9.6/workflow-linux/src/algorithm
        ${CMAKE_CURRENT_SOURCE_DIR}/workflow-0.9.6/workflow-linux/src/client
        ${CMAKE_CURRENT_SOURCE_DIR}/workflow-0.9.6/workflow-linux/src/factory
        ${CMAKE_CURRENT_SOURCE_DIR}/workflow-0.9.6/workflow-linux/src/kernel
        ${CMAKE_CURRENT_SOURCE_DIR}/workflow-0.9.6/workflow-linux/src/manager
        ${CMAKE_CURRENT_SOURCE_DIR}/workflow-0.9.6/workflow-linux/src/nameservice
        ${CMAKE_CURRENT_SOURCE_DIR}/workflow-0.9.6/workflow-linux/src/protocol
        ${CMAKE_CURRENT_SOURCE_DIR}/workflow-0.9.6/workflow-linux/src/server
        ${CMAKE_CURRENT_SOURCE_DIR}/workflow-0.9.6/workflow-linux/src/util
    )
endif()

# add dependency
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/openssl-1.1.1g/include
)

if (UNIX)
    # for linux kafka support
    include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/snappy-1.1.9
        ${CMAKE_CURRENT_SOURCE_DIR}/snappy-1.1.9/prebuild/linux
        ${CMAKE_CURRENT_SOURCE_DIR}/lz4-1.9.3/lib
        ${CMAKE_CURRENT_SOURCE_DIR}/zstd-1.5.0/lib
    )
endif()

if (WIN32)
    # windows use self build lib
    link_directories(
        openssl-1.1.1g/prebuild/win
    )
else()
    # linux use self build lib
    link_directories(
        openssl-1.1.1g/prebuild/linux
        snappy-1.1.9/prebuild/linux
        lz4-1.9.3/prebuild/linux
        zstd-1.5.0/prebuild/linux
    )
endif()

# specify workflow necessary source to compile lib
if (WIN32)
    file(GLOB WORKFLOW_SRC
        workflow-0.9.6/workflow-windows/src/algorithm/*.h
        workflow-0.9.6/workflow-windows/src/algorithm/*.cc
        workflow-0.9.6/workflow-windows/src/algorithm/*.inl
        workflow-0.9.6/workflow-windows/src/client/*.h
        workflow-0.9.6/workflow-windows/src/client/*.cc
        workflow-0.9.6/workflow-windows/src/factory/*.h
        workflow-0.9.6/workflow-windows/src/factory/*.cc
        workflow-0.9.6/workflow-windows/src/factory/*.inl
        workflow-0.9.6/workflow-windows/src/kernel_win/*.h
        workflow-0.9.6/workflow-windows/src/kernel_win/*.c
        workflow-0.9.6/workflow-windows/src/kernel_win/*.cc
        workflow-0.9.6/workflow-windows/src/manager/*.h
        workflow-0.9.6/workflow-windows/src/manager/*.cc
        workflow-0.9.6/workflow-windows/src/manager/*.inl
        workflow-0.9.6/workflow-windows/src/nameservice/*.h
        workflow-0.9.6/workflow-windows/src/nameservice/*.cc
        workflow-0.9.6/workflow-windows/src/protocol/*.h
        workflow-0.9.6/workflow-windows/src/protocol/*.c
        workflow-0.9.6/workflow-windows/src/protocol/*.cc
        workflow-0.9.6/workflow-windows/src/protocol/*.inl
        workflow-0.9.6/workflow-windows/src/server/*.h
        workflow-0.9.6/workflow-windows/src/server/*.cc
        workflow-0.9.6/workflow-windows/src/util/*.h
        workflow-0.9.6/workflow-windows/src/util/*.cc
        workflow-0.9.6/workflow-windows/src/*.h
    )
elseif (UNIX)
    file(GLOB WORKFLOW_SRC
        workflow-0.9.6/workflow-linux/src/algorithm/*.h
        workflow-0.9.6/workflow-linux/src/algorithm/*.cc
        workflow-0.9.6/workflow-linux/src/algorithm/*.inl
        workflow-0.9.6/workflow-linux/src/client/*.h
        workflow-0.9.6/workflow-linux/src/client/*.cc
        workflow-0.9.6/workflow-linux/src/factory/*.h
        workflow-0.9.6/workflow-linux/src/factory/*.cc
        workflow-0.9.6/workflow-linux/src/factory/*.inl
        workflow-0.9.6/workflow-linux/src/kernel/*.h
        workflow-0.9.6/workflow-linux/src/kernel/*.c
        workflow-0.9.6/workflow-linux/src/kernel/*.cc
        workflow-0.9.6/workflow-linux/src/manager/*.h
        workflow-0.9.6/workflow-linux/src/manager/*.cc
        workflow-0.9.6/workflow-linux/src/manager/*.inl
        workflow-0.9.6/workflow-linux/src/nameservice/*.h
        workflow-0.9.6/workflow-linux/src/nameservice/*.cc
        workflow-0.9.6/workflow-linux/src/protocol/*.h
        workflow-0.9.6/workflow-linux/src/protocol/*.c
        workflow-0.9.6/workflow-linux/src/protocol/*.cc
        workflow-0.9.6/workflow-linux/src/protocol/*.inl
        workflow-0.9.6/workflow-linux/src/server/*.h
        workflow-0.9.6/workflow-linux/src/server/*.cc
        workflow-0.9.6/workflow-linux/src/util/*.h
        workflow-0.9.6/workflow-linux/src/util/*.cc
        workflow-0.9.6/workflow-linux/src/*.h
    )
endif()

if (WIN32)
    add_library(workflow 
        ${WORKFLOW_SRC}
        )

    # must specify the macro in windows
    target_compile_definitions(
		workflow PRIVATE
		strdup=_strdup
		strcasecmp=_stricmp
		strncasecmp=_strnicmp
	)

    target_link_libraries(workflow 
        # link to prebuild lib, remember to copy dll to bin dir
        libssl.lib 
        libcrypto.lib

        # link behind
        ws2_32 
        wsock32 
    )

elseif (UNIX)
    add_library(workflow 
        ${WORKFLOW_SRC}
        )

    target_link_libraries(workflow 
        # link to prebuild lib
        libssl.a
        libcrypto.a
        libsnappy.a
        liblz4.so
        libzstd.a

        # link behind
        pthread 
        dl
    )
endif ()

# --- build self demo --- #
# -- main
# add source code
set(SRC 
    src/message.h
    src/message.cc
    src/main.cpp
)

# generate binary
add_executable(main ${SRC})

# link the whole workflow lib
target_link_libraries(main workflow)

# -- client
set(CLIENT_SRC
    src/message.h
    src/message.cc
    src/client.cc
)

add_executable(client ${CLIENT_SRC})

target_link_libraries(client workflow)

