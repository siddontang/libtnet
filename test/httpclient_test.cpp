#include <iostream>

#include "httpclient.h"

#include "httprequest.h"
#include "httpresponse.h"
#include "ioloop.h"

using namespace std;
using namespace tnet;

void onResponse(IOLoop* loop, const HttpResponse& resp)
{
    cout << resp.statusCode << endl;
    
    Headers_t::const_iterator iter = resp.headers.begin();

    while(iter != resp.headers.end())
    {
        cout << iter->first << ": " << iter->second << endl;
        ++iter;    
    }

    cout << resp.body.size() << endl << endl;    

    loop->stop();
}

int main()
{
    IOLoop loop;
    
    HttpClientPtr_t client = std::make_shared<HttpClient>(&loop);
 
    Headers_t headers;
    headers.insert(make_pair("Accept", "*/*"));
    headers.insert(make_pair("User-Agent", "curl/7.22.0 (x86_64-pc-linux-gnu) libcurl/7.22.0 OpenSSL/1.0.1 zlib/1.2.3.4 libidn/1.23 librtmp/2.3"));

    client->request("http://127.0.0.1:11181/abc", std::bind(&onResponse, &loop, _1));
    
    loop.start(); 

    cout << "exit" << endl;
    
    return 0;   
}
