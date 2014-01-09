#include "timingwheel.h"

#include "log.h"
#include "timer.h"

using namespace std;

namespace tnet 
{
    void dummyHandler(const TimingWheelPtr_t&)
    {
    
    }

    TimingWheel::TimingWheel(int interval, int maxBuckets)
        : m_loop(0)
        , m_interval(interval)
        , m_maxBuckets(maxBuckets)
        , m_nextBucket(0)
    {
        m_maxTimeout = interval * maxBuckets;

        m_timer = std::make_shared<Timer>(std::bind(&TimingWheel::onTimer, this, _1), interval, 0); 
   
        m_buckets.resize(maxBuckets);
    }

    TimingWheel::~TimingWheel()
    {
    
    }

    void TimingWheel::start(IOLoop* loop)
    {
        if(m_running)
        {
            LOG_WARN("timing wheel was started");
            return; 
        }   

        m_loop = loop;
        m_running = true;

        m_timer->start(loop);
    }

    void TimingWheel::stop()
    {
        if(!m_running)
        {
            LOG_WARN("timing wheel was stopped");
            return;
        }

        m_running = false;
        m_timer->stop();
    }

    void TimingWheel::onTimer(const TimerPtr_t& timer)
    {
        int index = m_nextBucket;

        auto& chans = m_buckets[index];
        for(auto iter = chans.begin(); iter != chans.end(); ++iter)
        {
            (*iter)(shared_from_this());
        }

        chans.clear();

        m_nextBucket = (m_nextBucket + 1) % m_maxBuckets;
    }

    union Slot
    {
        uint64_t h;
        uint32_t p[2];
    };

    uint64_t TimingWheel::add(const TimingWheelHandler_t& handler, int timeout)
    {
        if(timeout >= m_maxTimeout)
        {
            LOG_ERROR("timeout %d > max %d", timeout, m_maxTimeout);
            return (uint64_t)(-1);
        }

        uint32_t bucket = (m_nextBucket + timeout / m_interval) % m_maxBuckets;
        uint32_t id = uint32_t(m_buckets[bucket].size());
    
        m_buckets[bucket].push_back(handler);
   
        Slot u;
        u.p[0] = bucket;
        u.p[1] = id;
        return u.h;
    }

    uint64_t TimingWheel::update(uint64_t slot, int timeout) 
    {
        Slot u;
        u.h = slot;

        uint32_t bucket = u.p[0];
        uint32_t id = u.p[1];

        if(bucket > m_maxBuckets)
        {
            return (uint64_t)-1;
        }

        auto& chans = m_buckets[bucket];
        if(id >= uint32_t(chans.size())) 
        {
            return (uint64_t)-1;
        }
    
        auto&& cb = std::move(chans[id]);
        chans[id] = std::bind(&dummyHandler, _1);

        return add(cb, timeout);
    }

    void TimingWheel::remove(uint64_t slot)
    {
        Slot u;
        u.h = slot;

        uint32_t bucket = u.p[0];
        uint32_t id = u.p[1];

        if(bucket > m_maxBuckets)
        {
            return;
        }

        auto& chans = m_buckets[bucket];
        
        if(id >= uint32_t(chans.size()))
        {
            return;
        }

        chans[id] = std::bind(&dummyHandler, _1);
        
        return;
    }
}
