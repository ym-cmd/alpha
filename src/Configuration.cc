#include "../include/Configuration.h"
#include "../include/nlohmann/json.hpp"

#include <fstream>

using std::ifstream;

const string ConfigFilePath = "../conf/myconf.json";

Configuration* Configuration::_pInstance = nullptr;
pthread_once_t Configuration::_once = PTHREAD_ONCE_INIT;

Configuration* Configuration::getInstance() {
    pthread_once(&_once, init_r);
    return _pInstance;
}

unordered_map<string, string>& Configuration::getConfigMap() {
    return _configs;
}

// 用于返回对应的config路径，找到了则返回正确值，否则返回空string
const string Configuration::getConfigValStr(const string& configkeystr) const {
    auto it = _configs.find(configkeystr); 
    if (it != _configs.end()) {
        return it->second;
    } else {
        return "";
    }
}

Configuration::Configuration(const string configFilePath) : _configFilePath(configFilePath) {
    ifstream ifs(_configFilePath);

    if (!ifs.is_open()) {
        std::cerr << "Configuration Failed to open configPath\n";
        exit(1);
    }

    // 打开文件成功
    nlohmann::json config_json;
    ifs >> config_json;

    for (nlohmann::json::iterator it = config_json.begin(); it != config_json.end(); ++it) {
        _configs[it.key()] = it.value().get<std::string>();
    }    
    
    // 打印
    // for (const auto& entry : _configs) {
    //     std::cout << entry.first << ": " << entry.second << std::endl;
    // }

    ifs.close();
}

void Configuration::destroy() {
    if (_pInstance) {
        delete _pInstance;
        _pInstance = nullptr;
    }
    // std::cout << "Configuration destroy\n";
}

// init_r在pthread_once, 只会被执行一次
void Configuration::init_r() {
    _pInstance = new Configuration(ConfigFilePath);
    atexit(destroy); // 注册destroy()，结束后自动调用进行析构
}