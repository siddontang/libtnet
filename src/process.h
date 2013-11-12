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
        
        void start(size_t num);
        void stop();
        void restart();

        //same as waitpid, return dead children num
        int wait();
        
        bool isMainProc() { return m_main == getpid(); }
        
        bool hasChild() { return m_children.size() > 0; }


    private:
        pid_t m_main;
        std::set<pid_t> m_children;
        size_t m_childNum;
    };
    
}
