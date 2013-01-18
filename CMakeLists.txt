project(common CXX C)
cmake_minimum_required(VERSION 2.8)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0015 NEW)
endif(COMMAND cmake_policy)

set(COMMON_VERSION_MAJOR 1)
set(COMMON_VERSION_MINOR 0)
set(COMMON_VERSION_PATCH 0)
set(common_version ${COMMON_VERSION_MAJOR}.${COMMON_VERSION_MINOR})

if(WIN32)
    add_definitions("/W3 /wd4996 /wd4290")
endif(WIN32)

add_subdirectory(src)
add_subdirectory(src/gtest)
add_subdirectory(test)

set(CPACK_PACKAGE_NAME "common")
set(CPACK_PACKAGE_VENDOR "wilbur lang<wilburlang@gmail.com>")
set(CPACK_PACKAGE_SUMMARY "generic c++ lightweight wrapper")
set(CPACK_PACKAGE_VERSION "${common_version}")
set(CPACK_PACKAGE_VERSION_MAJOR "${COMMON_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${COMMON_VErSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${COMMON_VERSIOn_PATCH}")

include(CPack)


