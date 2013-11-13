set(CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}")

set (SRCS
    http_parser.c
    httpconnection.cpp
    httprequest.cpp
    httpresponse.cpp
    httpserver.cpp
    httputil.cpp
    wsconnection.cpp
    )

include_directories(../)

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

add_library(tnet_http ${SRCS})
target_link_libraries(tnet_http tnet)

install(TARGETS tnet_http DESTINATION lib)

set (HEADERS
    httpconnection.h
    http_parser.h
    httprequest.h
    httpresponse.h
    httpserver.h
    httputil.h
    tnet_http.h
    wsconnection.h
    )

install(FILES ${HEADERS} DESTINATION include/tnet/http)
   
