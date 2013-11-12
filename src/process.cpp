#include "process.h"

#include <sys/wait.h>
#include <sys/types.h>

#include "log.h"

using namespace std;

namespace tnet
{
    Process::Process()
    {
        m_main = getpid();     
    }

    Process::~Process()
    {
        
    }

    void Process::start(size_t num)
    {
        m_childNum = num;
        for(size_t i = 0; i < num; ++i)
        {
            pid_t pid = fork();
            if(pid < 0)
            {
                LOG_ERROR("fork error %s", errorMsg(errno));    
                continue;
            }    
            else if(pid == 0)
            {
                //child
                m_children.clear();
                return;    
            }
            else
            {
                //parent 
                m_children.insert(pid);    
            }
        }

        return;
    }

    void Process::stop()
    {
        for_each_all(m_children, std::bind(kill, _1, SIGTERM));    
    }

    void Process::restart()
    {
        size_t deads = m_childNum - m_children.size();    
        if(deads > 0)
        {
            start(deads);    
        }
    }

    int Process::wait()
    {
        pid_t pid;
        int status = 0;

        int deads = 0;
        while((pid = waitpid(-1, &status, WNOHANG)) > 0)
        {
            LOG_INFO("child was dead %d", pid);
            
            m_children.erase(pid);
            ++deads;
        }

        return deads;
    }
}
