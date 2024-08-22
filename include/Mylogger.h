#include "log4cpp/Category.hh"
#include "log4cpp/Appender.hh"
#include "log4cpp/FileAppender.hh"
#include "log4cpp/OstreamAppender.hh"
#include "log4cpp/Layout.hh"
#include "log4cpp/PatternLayout.hh"
#include "log4cpp/BasicLayout.hh"
#include "log4cpp/Priority.hh"
#include <string.h>
#include <string>
#include <iostream>
using namespace log4cpp;
using std::string;
#define addprefix(msg) string("[").append(__FILE__).append(" : ")    \
                        .append(__func__).append("()").append(" : ") \
                        .append(std::to_string(__LINE__)).append("] ").append(msg).c_str()

#define LogInfo(msg) Mylogger::getInstance()->info(addprefix(msg))
#define LogWarn(msg) Mylogger::getInstance()->warn(addprefix(msg))
#define LogError(msg) Mylogger::getInstance()->error(addprefix(msg))
#define LogDebug(msg) Mylogger::getInstance()->debug(addprefix(msg))

class Mylogger
{
public:
    static Mylogger* getInstance();

	void warn(const char *msg);
	void error(const char *msg);
	void debug(const char *msg);
	void info(const char *msg);

private:
    
    static void destroy();
    static void init_r();

	Mylogger();
	~Mylogger();
    Mylogger(const Mylogger&) = delete;
    Mylogger& operator=(const Mylogger&) = delete;

    
private:
    static pthread_once_t _once;
    static Mylogger* _pInstance;
    Category& _mycat;
};