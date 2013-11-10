set(CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}")

include_directories(${TNET_SOURCE_DIR}/src)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

add_executable(echoserver_test echoserver_test.cpp)
target_link_libraries(echoserver_test tnet)


