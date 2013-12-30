#include <iostream>
#include <string>
#include <stdlib.h>
#include <vector>

#include "log.h"
#include "timingwheel.h"
#include "httpclient.h"

#include "httprequest.h"
#include "httpresponse.h"
#include "ioloop.h"

using namespace std;
using namespace tnet;

string url = "http://10.20.187.120:11181/";

void onResponse(const HttpClientPtr_t& client, const HttpResponse& resp)
{
    if(resp.statusCode != 200)
    {
        cout << "request error:" << resp.statusCode << "\t" << resp.body << endl;
        return;   
    }

    client->request(url, std::bind(&onResponse, client, _1));
} 

void request(const TimingWheelPtr_t& wheel, const HttpClientPtr_t& client, int num)
{
    for(int i = 0; i < num; ++i)
    {
        client->request(url, std::bind(&onResponse, client, _1));
    }
}

int main(int argc, char* args[])
{
    Log::rootLog().setLevel(Log::ERROR);
  
    if(argc < 2)
    {
        cout << "args: num [eth0]" << endl;
    }

    int num = num = atoi(args[1]);

    vector<HttpClientPtr_t> clients;

    IOLoop loop;
    
    if(argc == 2)
    {
        clients.push_back(std::make_shared<HttpClient>(&loop));
    }
    else
    {
        for(int i = 0; i < argc - 2; ++i)
        {
            HttpClientPtr_t client = std::make_shared<HttpClient>(&loop);
            client->bindDevice(args[i + 2]);
            clients.push_back(client);
        }
    }

    cout << "request num:" << num << endl;


    TimingWheelPtr_t wheel = std::make_shared<TimingWheel>(1000, 3600);

    int c = 60 * clients.size();
    for(int i = 0; i < c; ++i)
    {
        int reqNum = num / c;
        for(auto it = clients.begin(); it != clients.end(); ++it)
        {
            wheel->add(std::bind(&request, _1, *it, reqNum), i * 1000);
        }
    }

    wheel->start(&loop);

    loop.start();

    return 0;
}
