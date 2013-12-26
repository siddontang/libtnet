set(CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}")

include_directories(../src)
include_directories(../src/http)
include_directories(../src/redis)

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

add_executable(timer_test timer_test.cpp)
target_link_libraries(timer_test tnet rt)

add_executable(timingwheel_test timingwheel_test.cpp)
target_link_libraries(timingwheel_test tnet rt)

add_executable(signaler_test signaler_test.cpp)
target_link_libraries(signaler_test tnet)

add_executable(notifier_test notifier_test.cpp)
target_link_libraries(notifier_test tnet)

add_executable(redisclient_test redisclient_test.cpp)
target_link_libraries(redisclient_test tnet tnet_redis)


add_executable(cometserver_test cometserver_test.cpp)
target_link_libraries(cometserver_test tnet tnet_http)


add_executable(cometclient_test cometclient_test.cpp)
target_link_libraries(cometclient_test tnet tnet_http)
