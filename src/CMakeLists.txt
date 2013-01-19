
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

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_custom_command(OUTPUT ${LIBRARY_OUTPUT_PATH}/libluajit.a
        COMMAND make
        COMMAND cp libluajit.a ${LIBRARY_OUTPUT_PATH}/.
        COMMAND cp luajit ${EXECUTABLE_OUTPUT_PATH}/.
        COMMAND make clean
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lua/jit/LuaJIT-2.0.0/src
        COMMENT "makeing luajit")
    add_custom_target(cxxliblua ALL DEPENDS ${LIBRARY_OUTPUT_PATH}/libluajit.a)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

set(COMMON_SRC
    ../common/config.h
    ../common/datetime.h datetime.cpp
    ../common/lua/scriptbind.h ../common/lua/scriptcall.h 
    ../common/lua/scriptutil.h ../common/lua/scriptvm.h
    lua/classhelper.cpp lua/scriptbind.cpp lua/scriptcall.cpp
    lua/scriptgc.cpp lua/scriptstack.cpp lua/scriptvm.cpp
    ../common/ipc/mmap.h ipc/file_mapping.cpp ipc/mapped_region.cpp
    ipc/shared_memory.cpp
    ../common/str/tokenizer.h ../common/str/trim.h 
    ../common/str/regex.h ../common/str/compare.h str/tokenizer.cpp
    )

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
else(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	add_definitions(-fPIC)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

link_directories(../../lib/${CMAKE_BUILD_TYPE})
add_library(common STATIC ${COMMON_SRC})

set_target_properties(common PROPERTIES VERSION ${common_version})
