#include <string>

#include "wsclient.h"
#include "ioloop.h"
#include "wsconnection.h"
#include "log.h"

using namespace std;
using namespace tnet;

int i = 0;

void onWriteComplete(int j)
{
    LOG_INFO("write complete %d", j);    
}

void onWsConnEvent(const WsConnectionPtr_t& conn, WsEvent event, const void* context)
{
    switch(event)
    {
        case Ws_OpenEvent:
            LOG_INFO("open");
            conn->send("Hello world");
            break;    
        case Ws_CloseEvent:
            LOG_INFO("close");
            break;
        case Ws_MessageEvent:
            {
                const string& msg = *(const string*)context;
                
                LOG_INFO("message %s", msg.c_str());

                char buf[1024];
                int n = snprintf(buf, sizeof(buf), "Hello World %d", i++);
                    
                conn->send(string(buf, n), std::bind(&onWriteComplete, i));
            
                if(i > 10)
                {
                    conn->close();    
                }
                
            }
            break;
        case Ws_ErrorEvent:
            LOG_INFO("error");
            break;
        default:
            break;
    }    
}

int main()
{
    IOLoop loop;
    WsClientPtr_t client = std::make_shared<WsClient>(&loop);
    
    client->connect("ws://127.0.0.1:11181/push/ws", std::bind(&onWsConnEvent, _1, _2, _3));
    
    loop.start();
    
    return 0;    
}
