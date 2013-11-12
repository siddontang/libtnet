#pragma once

#include <unistd.h>
#include <set>

#include "tnet.h"

namespace tnet
{

    class Process
    {
    public:
        Process();
        ~Process();
        
        void wait(size_t num, const ProcessCallback_t& callback);

        void kill();

        bool isMainProc() { return m_main == getpid(); }
        
        bool hasChild() { return m_children.size() > 0; }

    private:
        pid_t create();

    private:
        pid_t m_main;
        std::set<pid_t> m_children;
    };
    
}
