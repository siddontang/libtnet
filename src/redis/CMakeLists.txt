set(CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}")

set (SRCS
    hiredis.c
    redisclient.cpp
    redisconnection.cpp
    redistrans.cpp
    sds.c
    )

include_directories(../)

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

add_library(tnet_redis ${SRCS})
target_link_libraries(tnet_redis tnet)

install(TARGETS tnet_redis DESTINATION lib)

set (HEADERS
    hiredis.h
    redisclient.h
    redistrans.h
    tnet_redis.h
    )

install(FILES ${HEADERS} DESTINATION include/tnet/redis)
   
