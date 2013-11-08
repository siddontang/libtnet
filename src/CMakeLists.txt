set(CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}")

set (SRCS
    acceptor.cpp
    connection.cpp
    log.cpp
    ioloop.cpp
    poller.cpp
    sockutil.cpp
    timer.cpp
    )

add_library(tnet ${SRCS})

install(TARGETS tnet DESTINATION lib)

set (HEADERS
    )

install(FILES ${HEADERS} DESTINATION include/tnet)
