#include "signaler.h"

#include <algorithm>

#include <sys/signalfd.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

#include "log.h"
#include "ioloop.h"

using namespace std;

namespace tnet
{
    Signaler::Signaler(int signum, const SignalHandler_t& handler)
        : m_loop(0)
        , m_fd(-1)
        , m_running(false)
        , m_signums{signum}
        , m_handler(handler)
    {
        m_fd = createSignalFd(m_signums);
    }

    Signaler::Signaler(const vector<int>& signums, const SignalHandler_t& handler)
        : m_loop(0)
        , m_fd(-1)
        , m_running(false)
        , m_signums(signums)
        , m_handler(handler)
    {
        m_fd = createSignalFd(m_signums);
    }

    Signaler::~Signaler()
    {
        LOG_INFO("destroyed %d", m_fd);
        if(m_fd > 0)
        {
            close(m_fd);    
        } 
    } 
  
    int Signaler::createSignalFd(const std::vector<int>& signums)
    {
        sigset_t mask;
        
        sigemptyset(&mask);
        for(size_t i = 0; i < signums.size(); ++i)
        {
            sigaddset(&mask, signums[i]);
        }

        if(sigprocmask(SIG_BLOCK, &mask, NULL) < 0)
        {
            LOG_ERROR("sigprocmask error");
            return -1;  
        } 

        int fd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
        if(fd < 0)
        {
            LOG_ERROR("signalfd error %s", errorMsg(errno));    
        }
   
        return fd;
    }
   
    void Signaler::start(IOLoop* loop)
    {
        assert(m_fd > 0);
        if(m_running)
        {
            LOG_WARN("signaler was started");
            return;    
        }
        
        LOG_INFO("start signaler %d", m_fd);
        
        m_running = true;
        m_loop = loop;

        m_loop->addHandler(m_fd, TNET_READ, 
            std::bind(&Signaler::onSignal, shared_from_this(), _1, _2));
    }
    
    void Signaler::stop()
    {
        assert(m_fd > 0);
        if(!m_running)
        {
            LOG_WARN("signaler was stopped");
            return;    
        }

        LOG_INFO("stop signaler %d", m_fd);

        m_running = false;

        m_loop->removeHandler(m_fd);
    }     

    void Signaler::onSignal(IOLoop* loop, int events)
    {
        SignalerPtr_t signaler = shared_from_this();

        struct signalfd_siginfo fdsi;
        ssize_t s = read(m_fd, &fdsi, sizeof(fdsi));
        if(s != sizeof(fdsi))
        {
            LOG_ERROR("onSignal read error");
            return;    
        } 

        int signum = fdsi.ssi_signo;
        if(std::find(m_signums.begin(), m_signums.end(), signum) == m_signums.end())
        {
            LOG_ERROR("uexpect signum %d", signum);
            return;  
        } 
    
        m_handler(signaler, signum);
    }
}
