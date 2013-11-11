#include "log.h"

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

namespace tnet
{
    __thread char errnoMsgBuf[1024];

    const char* errorMsg(int code)
    {
        return strerror_r(code, errnoMsgBuf, sizeof(errnoMsgBuf));
    }

    static int MaxLogMsg = 1024;

    static const char* LevelMsg[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};

#define WRITE_LOG(level)\
    if(m_level <= level)\
    {\
        char msg[MaxLogMsg];\
        va_list ap;\
        va_start(ap, fmt);\
        vsnprintf(msg, sizeof(msg), fmt, ap);\
        va_end(ap);\
        log(LevelMsg[int(level)], file, function, line, msg);\
    }



    static const char* DateTimeFormat = "%Y-%m-%d %H:%M:%S";

    Log::Log()
    {
        m_fd = stdout;    
        m_level = TRACE;
    }
    
    Log::Log(const char* fileName)
    {
        m_fd = fopen(fileName, "ab+");
        m_level = TRACE;
    }       

    Log::~Log()
    {
        if(m_fd != stdout)
        {
            fclose(m_fd);    
        }
    }

    void Log::redirect(const char* fileName)
    {
        if(m_fd != stdout)
        {
            fclose(m_fd);
            m_fd = fopen(fileName, "ab+");    
        }    
    }

    void Log::trace(const char* file, const char* function, int line, const char* fmt, ...)
    {
        WRITE_LOG(TRACE);
    }

    void Log::debug(const char* file, const char* function, int line, const char* fmt, ...)
    {
        WRITE_LOG(DEBUG);
    }

    void Log::info(const char* file, const char* function, int line, const char* fmt, ...)
    {
        WRITE_LOG(INFO);
    }

    void Log::warn(const char* file, const char* function, int line, const char* fmt, ...)
    {
        WRITE_LOG(WARN);
    }

    void Log::error(const char* file, const char* function, int line, const char* fmt, ...)
    {
        WRITE_LOG(ERROR);
    }

    void Log::fatal(const char* file, const char* function, int line, const char* fmt, ...)
    {
        WRITE_LOG(FATAL);
    }

    void Log::log(const char* level, const char* file, const char* function, int line, const char* msg)
    {
        char buf[64];

        time_t now = time(NULL);
        strftime(buf, sizeof(buf), DateTimeFormat, gmtime(&now));

        fprintf(m_fd, "%s %s [%d] %s %s:%d %s\n", buf, level, getpid(), function, file, line, msg);    
    }
}
