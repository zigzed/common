if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(PTHREAD pthread)
    set(DL	dl)
    set(RT	rt)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

include_directories(../)
link_directories(../lib/${CMAKE_BUILD_TYPE})
set(EXECUTABLE_OUTPUT_PATH ../../bin/${CMAKE_BUILD_TYPE})
set(LIBRARY_OUTPUT_PATH ../../bin/${CMAKE_BUILD_TYPE})

add_executable(test_datetime test_datetime.cpp)
target_link_libraries(test_datetime common gtest ${PTHREAD})

