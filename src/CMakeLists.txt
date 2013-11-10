set(CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}")

set (SRCS
    acceptor.cpp
    address.cpp
    connection.cpp
    log.cpp
    ioloop.cpp
    poller.cpp
    sockutil.cpp
    tcpserver.cpp
    timer.cpp
    )

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

add_library(tnet ${SRCS})
target_link_libraries(tnet rt)

install(TARGETS tnet DESTINATION lib)

set (HEADERS
    )

install(FILES ${HEADERS} DESTINATION include/tnet)
