#include "notifier.h"

#include <sys/eventfd.h>
#include <assert.h>

#include "log.h"
#include "ioloop.h"

namespace tnet
{

    Notifier::Notifier(const NotifierHandler_t& handler)
        : m_loop(0)
        , m_fd(0)
        , m_running(false)
        , m_handler(handler)
    {
        m_fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if(m_fd < 0)
        {
            LOG_ERROR("eventfd error %s", errorMsg(errno));    
        }
    }

    Notifier::~Notifier()
    {
        LOG_INFO("destroyed %d", m_fd);
        
        if(m_fd > 0)
        {
            close(m_fd);    
        }    
    }

    void Notifier::start(IOLoop* loop)
    {
        assert(m_fd > 0);
        if(m_running)
        {
            LOG_WARN("event was started");
            return;    
        }    

        m_running = true;
        m_loop = loop;

        m_loop->addHandler(m_fd, TNET_READ, std::bind(&Notifier::onEvent, shared_from_this(), _1, _2));
    }    

    void Notifier::stop()
    {
        assert(m_fd > 0);
        if(!m_running)
        {
            LOG_WARN("event was stopped");
            return;    
        }    

        m_running = false;
        m_loop->removeHandler(m_fd);
    }
   
    void Notifier::notify()
    {
        eventfd_t value = 1;
        if(eventfd_write(m_fd, value) < 0)
        {
            LOG_ERROR("eventfd_write error");    
        }     
    }
    
    void Notifier::onEvent(IOLoop* loop, int events)
    {
        NotifierPtr_t notifier = shared_from_this();
        eventfd_t value;
        
        if(eventfd_read(m_fd, &value) < 0)
        {
            LOG_ERROR("eventfd read error");
            return;    
        }    

        m_handler(notifier);
    } 
}
