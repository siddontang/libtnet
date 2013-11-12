#include "process.h"

#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

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
        for(size_t i = 0; i < num; ++i)
        {
            pid_t pid = create();
            if(pid != 0)
            {
                callback();
                return;    
            }
        }

        while(1)
        {
            int status = 0;
            pid_t pid;
            if((pid = waitpid(-1, &status, WNOHANG)) > 0)
            {
                LOG_INFO("child was dead, restart it");
                m_children.erase(pid);
            
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

    void Process::kill()
    {
        for_each_all(m_children, std::bind(::kill, _1, SIGTERM));    
    }

}
