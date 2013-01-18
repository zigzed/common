project(common CXX C)
cmake_minimum_required(VERSION 2.8)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0015 NEW)
endif(COMMAND cmake_policy)

set(CXXLIB_VERSION_MAJOR 1)
set(CXXLIB_VERSION_MINOR 0)
set(CXXLIB_VERSION_PATCH 0)
set(cxxlib_version ${CXXLIB_VERSION_MAJOR}.${CXXLIB_VERSION_MINOR})

if(WIN32)
    add_definitions("/W3 /wd4996 /wd4290")
endif(WIN32)

#add_subdirectory(src)
add_subdirectory(src/gtest)
#add_subdirectory(test)

set(CPACK_PACKAGE_NAME "cxxlib")
set(CPACK_PACKAGE_VENDOR "wilbur lang<wilburlang@gmail.com>")
set(CPACK_PACKAGE_SUMMARY "generic c++ lightweight wrapper")
set(CPACK_PACKAGE_VERSION "${cxxlib_version}")
set(CPACK_PACKAGE_VERSION_MAJOR "${CXXLIB_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${CXXLIB_VErSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${CXXLIB_VERSIOn_PATCH}")

include(CPack)


