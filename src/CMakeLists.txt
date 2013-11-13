set(CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}")

set (SRCS
    acceptor.cpp
    address.cpp
    connchecker.cpp
    connection.cpp
    notifier.cpp
    log.cpp
    ioloop.cpp
    poller.cpp
    process.cpp
    signaler.cpp
    sockutil.cpp
    stringutil.cpp
    tcpserver.cpp
    timer.cpp

    polarssl/base64.c
    polarssl/md5.c
    polarssl/sha1.c
    )

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

add_library(tnet ${SRCS})
target_link_libraries(tnet rt)

install(TARGETS tnet DESTINATION lib)

set (HEADERS
    address.h
    connection.h
    log.h
    nocopyable.h
    notifier.h
    signaler.h
    sockutil.h
    spinlock.h
    stringutil.h
    tcpserver.h
    timer.h
    tnet.h 
    )

install(FILES ${HEADERS} DESTINATION include/tnet)


add_subdirectory(http)
