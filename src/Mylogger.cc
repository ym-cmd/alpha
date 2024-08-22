#include "../include/Mylogger.h"

pthread_once_t Mylogger::_once = PTHREAD_ONCE_INIT;
Mylogger* Mylogger::_pInstance = nullptr;

Mylogger::Mylogger() : _mycat(Category::getInstance("_mycat")) {
    auto ptn1 = new PatternLayout();
    ptn1->setConversionPattern("%d %c [%p] %m%n");

    auto ptn2 = new PatternLayout();
    ptn2->setConversionPattern("%d %c [%p] %m%n");

    auto pos = new OstreamAppender("console", &std::cout);
    pos->setLayout(ptn1);

    auto pfile = new FileAppender("default", "../log/Mylogger.log");
    pfile->setLayout(ptn2);

    _mycat.setPriority(Priority::INFO);
    _mycat.setAppender(pos);
    _mycat.setAppender(pfile);    
}

Mylogger::~Mylogger() {
    Category::shutdown();
    std::cout << "~Mylogger()" << std::endl;
}

Mylogger* Mylogger::getInstance() {
    pthread_once(&_once, init_r);
    return _pInstance;
}

void Mylogger::destroy() {
    if (_pInstance) {
        delete _pInstance;
        _pInstance = nullptr;
    }
}
void Mylogger::init_r() {
    _pInstance = new Mylogger();
    atexit(destroy);
}

void Mylogger::warn(const char *msg) {
    _mycat.warn(msg);
}

void Mylogger::error(const char *msg) {
    _mycat.error(msg);
}

void Mylogger::debug(const char *msg) {
    _mycat.debug(msg);
}

void Mylogger::info(const char *msg) {
    _mycat.info(msg);
}