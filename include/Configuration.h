#pragma once

#include <string>
#include <iostream>
#include <unordered_map>

using std::string; using std::unordered_map;

const string configFilePath = "../conf/myconf.json";

class Configuration {
public:
    static Configuration* getInstance();
    unordered_map<string, string>& getConfigMap();

private:
    Configuration(const string configFilePath);
    static void destroy();
    static void init_r();

    Configuration(const Configuration& rhs) = delete;
    Configuration& operator=(const Configuration& rhs) = delete;

private:
    static Configuration* _pInstance;
    static pthread_once_t _once;

    unordered_map<string, string> _configs;
    string _configFilePath;
};

Configuration* Configuration::_pInstance = nullptr;
pthread_once_t Configuration::_once = PTHREAD_ONCE_INIT;