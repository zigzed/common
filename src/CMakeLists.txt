
include_directories(../)

set(COMMON_ROOT ${CMAKE_SOURCE_DIR})
set(LIBRARY_OUTPUT_PATH ${COMMON_ROOT}/lib/${CMAKE_BUILD_TYPE})
set(EXECUTABLE_OUTPUT_PATH ${COMMON_ROOT}/bin/${CMAKE_BUILD_TYPE})

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_definitions(-DLUA_USE_LINUX)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
if(${CMAKE_SYSTEM_NAME} MATCHES "bsd")
    add_definitions(-DLUA_USE_POSIX)
endif(${CMAKE_SYSTEM_NAME} MATCHES "bsd")

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    add_definitions(-DLUA_ANSI)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

set(LUA51_SRC
    lua/lua/lapi.c lua/lua/lauxlib.c lua/lua/lbaselib.c lua/lua/lcode.c
    lua/lua/ldblib.c lua/lua/ldebug.c lua/lua/ldo.c lua/lua/ldump.c
    lua/lua/lfunc.c lua/lua/lgc.c lua/lua/linit.c lua/lua/liolib.c
    lua/lua/llex.c lua/lua/lmathlib.c lua/lua/lmem.c lua/lua/loadlib.c
    lua/lua/lobject.c lua/lua/lopcodes.c lua/lua/loslib.c
    lua/lua/lparser.c lua/lua/lstate.c
    lua/lua/lstring.c lua/lua/lstrlib.c lua/lua/ltable.c lua/lua/ltablib.c
    lua/lua/ltm.c lua/lua/lundump.c lua/lua/lvm.c
    lua/lua/lzio.c)
add_library(lua51lib STATIC ${LUA51_SRC})
add_executable(lua51 lua/lua/lua.c lua/lua/print.c)
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_link_libraries(lua51 lua51lib)
else(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_link_libraries(lua51 lua51lib m dl)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

set(COMMON_SRC
    ../common/config.h
    ../common/datetime.h datetime.cpp
    )

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
else(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	add_definitions(-fPIC)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

link_directories(../../lib/${CMAKE_BUILD_TYPE})
add_library(common STATIC ${COMMON_SRC})

set_target_properties(common PROPERTIES VERSION ${common_version})

