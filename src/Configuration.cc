#include "../include/Configuration.h"
#include <fstream>

using std::ifstream;

const string ConfigFilePath = "../conf/myconf.json";

Configuration* Configuration::_pInstance = nullptr;
pthread_once_t Configuration::_once = PTHREAD_ONCE_INIT;

Configuration* Configuration::getInstance() {

}
unordered_map<string, string>& Configuration::getConfigMap() {

}
Configuration::Configuration(const string configFilePath) : _configFilePath(configFilePath) {
    ifstream ifs(_configFilePath);
    if (!ifs.is_open()) {

    }

}

void Configuration::destroy() {
    if (_pInstance) {
        delete _pInstance;
        _pInstance = nullptr;
    }
    std::cout << "Configuration destroy\n";
}

// init_r在pthread_once, 只会被执行一次
void Configuration::init_r() {
    _pInstance = new Configuration(ConfigFilePath);
    atexit(destroy); // 注册destroy()，结束后自动调用进行析构
}
