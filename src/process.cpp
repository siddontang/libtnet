#include "process.h"

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#include "log.h"

using namespace std;

namespace tnet
{
    Process::Process()
        : m_running(false)
    {
        m_main = getpid();     
    }

    Process::~Process()
    {
        
    }

    pid_t Process::create()
    {
        pid_t pid = fork();
        if(pid < 0)
        {
            LOG_ERROR("fork error %s", errorMsg(errno));    
            return 0;
        }    
        else if(pid == 0)
        {
            //child
            m_children.clear();
            return getpid();    
        }
        else
        {
            //parent 
            m_children.insert(pid);    
        } 

        return 0;
    }

    void Process::wait(size_t num, const ProcessCallback_t& callback)
    {
        m_running = true;
        for(size_t i = 0; i < num; ++i)
        {
            pid_t pid = create();
            if(pid != 0)
            {
                callback();
                return;    
            }
        }

        while(!m_children.empty())
        {
            int status = 0;
            pid_t pid;
            if((pid = waitpid(-1, &status, WNOHANG)) > 0)
            {
                m_children.erase(pid);
                
                if(!m_running)
                {
                    continue;    
                }
                
                LOG_INFO("child was dead, restart it");
            
                if(create() != 0)
                {
                    callback();
                    return;    
                }
            }    
            else
            {
                sleep(1);
                continue;
            }
        }

        return;
    }

    void Process::stop()
    {
        m_running = false;
        for_each_all(m_children, std::bind(::kill, _1, SIGTERM));    
    }

}
