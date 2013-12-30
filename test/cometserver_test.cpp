#include "tnet.h"

#include <stdlib.h>

#include "log.h"
#include "tcpserver.h"
#include "address.h"
#include "httpserver.h"
#include "httprequest.h"
#include "httpresponse.h"
#include "httpconnection.h"
#include "timingwheel.h"

using namespace std;
using namespace tnet;

class CometServer
{
public:
    TimingWheelPtr_t wheel;
};

static CometServer comet;

void onServerRun(IOLoop* loop)
{
    comet.wheel = std::make_shared<TimingWheel>(1000, 3600);
    comet.wheel->start(loop);
}

void onTimeout(const TimingWheelPtr_t& wheel, const WeakHttpConnectionPtr_t& conn)
{
    HttpConnectionPtr_t c = conn.lock();
    if(c)
    {
        c->send(200);
    } 
}

void onHandler(const HttpConnectionPtr_t& conn, const HttpRequest& request)
{
    if(request.method == HTTP_GET)
    {
        int timeout = random() % 60 + 30;
        comet.wheel->add(std::bind(&onTimeout, _1, WeakHttpConnectionPtr_t(conn)), timeout * 1000);
        //conn->send(200);
    }
    else
    {
        conn->send(405);
    }
}

int main()
{
    Log::rootLog().setLevel(Log::ERROR);
    
    TcpServer s;
    
    s.setRunCallback(std::bind(&onServerRun, _1));
    
    HttpServer httpd(&s);

    httpd.setHttpCallback("/", std::bind(&onHandler, _1, _2));

    httpd.listen(Address(11181));

    s.start(8);

    return 0; 
}
