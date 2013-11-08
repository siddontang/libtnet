#ifndef _NOCOPYABLE_H_
#define _NOCOPYABLE_H_

namespace tnet
{

    class nocopyable
    {
    protected:
        nocopyable() {}
        ~nocopyable() {}
    private:  
        nocopyable(const nocopyable&);
        nocopyable& operator=(const nocopyable&); 
    };
    
}

#endif
