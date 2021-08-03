cmake_minimum_required(VERSION 3.6)

project(workflow_starter)

# --- compile workflow --- #
# definition
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
    )
endif()

# add dependency include dir
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/openssl-1.1.1g/include
)

# add link path for workflow dependency
if (WIN32)
    link_directories(
        openssl-1.1.1g/prebuild/win
    )
elseif (UNIX)
    link_directories(
        openssl-1.1.1g/prebuild/linux
    )
endif()

# specify workflow necessary source to compile
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
    set(WORKFLOW_SRC
        workflow-0.9.6/workflow-linux/*.*
    )
endif()

if (WIN32)
    add_library(workflow 
        ${WORKFLOW_SRC}

        #ws2_32 
        #wsock32 
        #libssl.lib 
        #libcrypto.lib
        )
elseif (UNIX)
    add_library(workflow 
        ${WORKFLOW_SRC}

        #pthread 
        #ssl 
        #crypto
        )
endif ()

# --- build self demo --- #
# add source code
set(SRC src/main.cpp)

# generate binary
add_executable(${PROJECT_NAME} ${SRC})

# link lib
target_link_libraries(${PROJECT_NAME} ${WORKFLOW_LIB})

