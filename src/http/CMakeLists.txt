set(CMAKE_CXX_FLAGS "-std=c++0x ${CMAKE_CXX_FLAGS}")

set (SRCS
    http_parser.c
    httpclient.cpp
    httpconnection.cpp
    httpconnector.cpp
    httpparser.cpp
    httprequest.cpp
    httpresponse.cpp
    httpserver.cpp
    httputil.cpp
    wsclient.cpp
    wsconnection.cpp
    wsutil.cpp
    )

add_definitions(-DHTTP_PARSER_STRICT=0)

include_directories(../)

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

add_library(tnet_http ${SRCS})
target_link_libraries(tnet_http tnet)

install(TARGETS tnet_http DESTINATION lib)

set (HEADERS
    httpclient.h
    httpconnection.h
    http_parser.h
    httpparser.h
    httprequest.h
    httpresponse.h
    httpserver.h
    httputil.h
    tnet_http.h
    wsclient.h
    wsconnection.h
    )

install(FILES ${HEADERS} DESTINATION include/tnet/http)
   
