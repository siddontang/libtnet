#include "tnet_http.h"
#include "httpserver.h"
#include "tcpserver.h"

#include "log.h"
#include "address.h"
#include "connection.h"
#include "wsconnection.h"

using namespace std;
using namespace tnet;

void onWsCallback(const WsConnectionPtr_t& conn, WsEvent event, const void* context)
{
    switch(event)
    {
        case Ws_OpenEvent:
            LOG_INFO("websocket open");
            break;    
        case Ws_CloseEvent:
            LOG_INFO("websocket close");
            break;
        case Ws_MessageEvent:
            {
                const string& str = *(const string*)context;
                LOG_INFO("websocket message %s", str.c_str());
                conn->send("hello " + str);
            }
            break;
        case Ws_PongEvent:
            LOG_INFO("websocket pong");
            break;
        case Ws_ErrorEvent:
            LOG_INFO("websocket error");
            break;
    }
}

int main()
{
    TcpServer s;
    
    HttpServer httpd(&s);
    
    httpd.setWsCallback("/push/ws", std::bind(&onWsCallback, _1, _2, _3));    

    httpd.listen(Address(11181));

    s.start();
    
    return 0; 
}

