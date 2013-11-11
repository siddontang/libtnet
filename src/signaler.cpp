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
    Signaler::Signaler(IOLoop* loop, int signum, const SignalHandler_t& handler)
        : m_loop(loop)
        , m_fd(-1)
        , m_handler(handler)
    {
        vector<int> signums(1, signum);
        resetFd(signums);
    }

    Signaler::Signaler(IOLoop* loop, const vector<int>& signums, const SignalHandler_t& handler)
        : m_loop(loop)
        , m_fd(-1)
        , m_handler(handler)
    {
        resetFd(signums);
    }

    Signaler::~Signaler()
    {
        if(m_fd > 0)
        {
            close(m_fd);    
        } 
    } 
   
    void Signaler::resetFd(const vector<int>& signums)
    {
        m_signums = signums;

        sigset_t mask;
        
        sigemptyset(&mask);
        for(size_t i = 0; i < m_signums.size(); ++i)
        {
            sigaddset(&mask, m_signums[i]);
        }

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

    void Signaler::start()
    {
        assert(m_fd > 0);
        LOG_INFO("signaler start %d", m_fd);

        m_loop->addHandler(m_fd, TNET_READ, 
            std::bind(&Signaler::onSignal, shared_from_this(), _1, _2));
    }
    
    void Signaler::stop()
    {
        LOG_INFO("signaler stop %d", m_fd);
        assert(m_fd > 0);

        m_loop->removeHandler(m_fd);
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

        int signum = fdsi.ssi_signo;
        if(std::find(m_signums.begin(), m_signums.end(), signum) == m_signums.end())
        {
            LOG_ERROR("uexpect signum %d", signum);
            return;  
        } 
    
        m_handler(signum);
    }
}
