#pragma once

#include <memory>
#include <functional>
#include <stdint.h>

#include "nocopyable.h"

using namespace std::placeholders;

namespace tnet
{
#define TNET_VERSION "0.1"

    class IOLoop;
    class Acceptor;
    class Connection;
    class Timer;
    class Signaler;
    class Process;
    class Notifier;
    class Address;
    class TimingWheel;

    enum 
    {
        TNET_NONE  = 0x00000000,
        TNET_READ  = 0x00000001,
        TNET_WRITE = 0x00000002,
        TNET_ERROR = 0x80000000,
    };

    enum ConnEvent
    {
        Conn_EstablishedEvent,
        Conn_ConnectEvent,
        Conn_ConnectingEvent,
        Conn_ReadEvent,
        Conn_WriteCompleteEvent,
        Conn_ErrorEvent,
        Conn_CloseEvent,
    };

    template<typename T, typename Func>
    void for_each_all(T& c, const Func& func)
    {
        typename T::iterator iter = c.begin();
        while(iter != c.end())
        {
            func(*iter);
            ++iter;
        }
    }

    typedef std::shared_ptr<Connection> ConnectionPtr_t;
    typedef std::weak_ptr<Connection> WeakConnectionPtr_t;
    typedef std::shared_ptr<Acceptor> AcceptorPtr_t;
    typedef std::shared_ptr<Timer> TimerPtr_t;
    typedef std::shared_ptr<Signaler> SignalerPtr_t;
    typedef std::shared_ptr<Process> ProcessPtr_t;
    typedef std::shared_ptr<Notifier> NotifierPtr_t;
    typedef std::shared_ptr<TimingWheel> TimingWheelPtr_t;

    typedef std::function<void (IOLoop*, int)> IOHandler_t;
    typedef std::function<void (IOLoop*, int)> NewConnCallback_t;
   
    typedef std::function<void (IOLoop*)> ServerRunCallback_t;
    
    class StackBuffer
    {
    public:
        StackBuffer(const char* buf, size_t c) : buffer(buf), count(c) {}
        
        const char* buffer;
        size_t count;    
    };

    //Conn_ReadEvent, context is StackBuffer, it will be destroied after callback
    //you must fetch data and store yourself if you need it later
    //other conn event, context is NULL
    
    typedef std::function<void (const ConnectionPtr_t&, ConnEvent, const void* context)> ConnEventCallback_t;

    typedef std::function<void ()> Callback_t;
    typedef std::function<void ()> ProcessCallback_t;

    typedef std::function<void (const TimerPtr_t&)> TimerHandler_t;

    typedef std::function<void (const SignalerPtr_t&, int)> SignalHandler_t;
    typedef std::function<void (const NotifierPtr_t&)> NotifierHandler_t;
    typedef std::function<void (const TimingWheelPtr_t&)> TimingWheelHandler_t;
}
