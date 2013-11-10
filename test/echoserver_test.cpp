#include <string>

#include "tnet.h"
#include "tcpserver.h"
#include "log.h"
#include "connection.h"
#include "address.h"

using namespace std;
using namespace tnet;

void onConnEvent(const ConnectionPtr_t& conn, ConnEvent event, void* context)
{
    switch(event)
    {
        case Conn_ReadEvent:
            {
                StackBuffer* buffer = static_cast<StackBuffer*>(context);
                conn->send(string(buffer->buffer, buffer->count));
            }
            break;
        default:
            break;
    }    
}

int main()
{
    TcpServer s;
    s.listen(Address(11181), std::bind(&onConnEvent, _1, _2, _3));

    s.start();

    return 0;
}
