
#include <hiredis/hiredis.h>

class Redis {
public:
    static Redis* getInstance();
    redisContext* getcontext() const;

private:  
    static void destroy();
    Redis();
    ~Redis();

    Redis(const Redis& rhs);
    Redis& operator=(const Redis& rhs);
private:
    static Redis* _pInstance;
    
    redisContext* _context;
};