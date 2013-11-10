#include "signaler.h"

#include <sys/signalfd.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include "log.h"
#include "ioloop.h"

namespace tnet
{
    Signaler::Signaler(IOLoop* loop, int signum, const SignalHandler_t& handler)
        : m_loop(loop)
        , m_signum(signum)
        , m_fd(0)
        , m_handler(handler)
    {
        sigset_t mask;
        
        sigemptyset(&mask);
        sigaddset(&mask, signum);
        
        if(sigprocmask(SIG_BLOCK, &mask, NULL) < 0)
        {
            LOG_ERROR("sigprocmask error");
            return;  
        } 

        m_fd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
        if(m_fd < 0)
        {
            LOG_ERROR("signalfd error %s", errorMsg(errno));    
        }
    }

    Signaler::~Signaler()
    {
    
    
    } 
   
    void Signaler::start()
    {
        assert(m_fd > 0);

        m_loop->addHandler(m_fd, TNET_READ, 
            std::bind(&Signaler::onSignal, this, _1, _2));
    }
    
    void Signaler::stop()
    {
        assert(m_fd > 0);

        m_loop->removeHandler(m_fd);
        close(m_fd);
        m_fd = 0;
    }     

    void Signaler::onSignal(IOLoop* loop, int events)
    {
        struct signalfd_siginfo fdsi;
        ssize_t s = read(m_fd, &fdsi, sizeof(fdsi));
        if(s != sizeof(fdsi))
        {
            LOG_ERROR("onSignal read error");
            return;    
        } 

        if(fdsi.ssi_signo != m_signum)
        {
            LOG_ERROR("uexpect signum, want %d but %d", m_signum, fdsi.ssi_signo);
            return;  
        } 
    
        m_handler(m_signum);
    }
}
