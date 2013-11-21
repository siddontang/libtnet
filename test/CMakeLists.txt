set(CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}")

include_directories(../src)
include_directories(../src/http)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

add_executable(echoserver_test echoserver_test.cpp)
target_link_libraries(echoserver_test tnet)

add_executable(echoclient_test echoclient_test.cpp)
target_link_libraries(echoclient_test tnet)

add_executable(httpserver_test httpserver_test.cpp)
target_link_libraries(httpserver_test tnet tnet_http)

add_executable(httpclient_test httpclient_test.cpp)
target_link_libraries(httpclient_test tnet tnet_http)

add_executable(wsserver_test wsserver_test.cpp)
target_link_libraries(wsserver_test tnet tnet_http)

add_executable(wsclient_test wsclient_test.cpp)
target_link_libraries(wsclient_test tnet tnet_http)
