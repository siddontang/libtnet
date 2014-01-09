#pragma once 

#include "tnet.h"

#include <vector>
#include <map>

namespace tnet 
{
    class TimingWheel : public nocopyable
                      , public std::enable_shared_from_this<TimingWheel>
    {
    public:
        //interval is milliseconds
        //max timeout is interval * maxBuckets
        TimingWheel(int interval, int maxBuckets);
        ~TimingWheel();

        void start(IOLoop* loop);
        void stop();

        IOLoop* loop() { return m_loop; }

        uint64_t add(const TimingWheelHandler_t& handler, int timeout);
        uint64_t update(uint64_t slot, int timeout);
        void remove(uint64_t slot);

    private:
        void onTimer(const TimerPtr_t& timer);

    private:
        IOLoop* m_loop;
        bool m_running;
        TimerPtr_t m_timer;

        int m_interval;
        int m_maxBuckets;
    
        int m_maxTimeout;
    
        int m_nextBucket;
        
        typedef std::vector<TimingWheelHandler_t> TimingChan_t;
        typedef std::vector<TimingChan_t> Buckets_t;

        Buckets_t m_buckets;
    }; 

}
