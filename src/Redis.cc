#include "../include/redis.h"
#include <stdlib.h>
Redis* Redis::_pInstance = Redis::getInstance();
Redis* Redis::getInstance() {
    if (_pInstance == nullptr) {
        atexit(destroy);
        _pInstance = new Redis();
    }
    return _pInstance;
}

void Redis::destroy() {
    if (_pInstance) {
        delete _pInstance;
        _pInstance = nullptr;
    }
    return;
}

Redis::Redis() {
    _context = redisConnect("127.0.0.1", 6379);
    if (_context == NULL || _context->err) {
    if (_context) {
        printf("Error: %s\n", _context->errstr);
        redisFree(_context);
    } else {
        printf("Can't allocate redis context\n");
    }
    exit(1);
}
}

Redis::~Redis() {
    redisFree(_context);
}

redisContext* Redis::getcontext() const{
    return _context;
}