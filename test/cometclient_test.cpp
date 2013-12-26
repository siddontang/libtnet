#include <iostream>
#include <string>
#include <stdlib.h>

#include "httpclient.h"

#include "httprequest.h"
#include "httpresponse.h"
#include "ioloop.h"

using namespace std;
using namespace tnet;

string url = "http://10.20.189.246:11181/";

void onResponse(const HttpClientPtr_t& client, const HttpResponse& resp)
{
    if(resp.statusCode != 200)
    {
        cout << "request error:" << resp.statusCode << "\t" << resp.body << endl;
        return;   
    }

    client->request(url, std::bind(&onResponse, client, _1));
} 

int main(int argc, char* args[])
{
    int num = 60000;
    if(argc == 2)
    {
        num = atoi(args[1]);
    }
        
    cout << "request num:" << num << endl;
    
    IOLoop loop;

    HttpClientPtr_t client = std::make_shared<HttpClient>(&loop);

    for(int i = 0; i < num; ++i)
    {
        client->request(url, std::bind(&onResponse, client, _1));
    }

    loop.start();

    return 0;
}
