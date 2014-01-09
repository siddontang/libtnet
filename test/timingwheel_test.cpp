#include <iostream>
#include <timer.h>
#include "timingwheel.h"
#include "ioloop.h"
#include "tnet.h"

using namespace std;
using namespace tnet;

int i = 0;

void onWheel(const TimingWheelPtr_t& wheel, int num)
{
    i++;
    
    cout << i << "\t" << num << "\t" << "onwheel\t" << time(0) << endl;
    uint64_t h = wheel->add(std::bind(&onWheel, _1, num), num * 1000);
    
    if(i == 5)
    {
        wheel->update(h, num * 1000);
    }

    if(i == 6)
    {
        wheel->remove(h);
        wheel->add(std::bind(&onWheel, _1, num), num * 1000);
    }

    if(i > 10)
    {
        wheel->loop()->stop();
    }
}

int main()
{
    IOLoop loop;

    TimingWheelPtr_t t = std::make_shared<TimingWheel>(1000, 20);

    t->add(std::bind(&onWheel, _1, 1), 1000);
    t->add(std::bind(&onWheel, _1, 2), 2000);

    t->start(&loop);

    cout << "start" << endl;
    loop.start();

    t->stop();

    cout << "end" << endl;

    return 0;
}
