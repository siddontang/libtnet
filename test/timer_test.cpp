#include <iostream>
#include <timer.h>

#include "timer.h"
#include "ioloop.h"
#include "tnet.h"

using namespace std;
using namespace tnet;

int i = 0;

void onTimer(const TimerPtr_t& timer)
{
    cout << "onTimer\t" << time(0) << endl;
    if(++i >= 10)
    {
        IOLoop* loop = timer->loop();
        
        timer->stop();
        
        loop->stop();    
    } 
}

void onOnceTimer(const TimerPtr_t& timer)
{
    cout << "onOnceTimer" << endl;    
}

void run(IOLoop* loop)
{
    TimerPtr_t timer1 = std::make_shared<Timer>(std::bind(&onTimer, _1), 1000, 1000);
    TimerPtr_t timer2 = std::make_shared<Timer>(std::bind(&onOnceTimer, _1), 0, 5000);

    timer1->start(loop);
    timer2->start(loop);
}

int main()
{

    IOLoop loop;

    run(&loop);

    cout << "start" << endl;

    loop.start();

    cout << "end" << endl;

    return 0;
    
}
