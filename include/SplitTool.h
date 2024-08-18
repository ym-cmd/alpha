#pragma once

#include "../include/simhash/simhash/cppjieba/Jieba.hpp"

#include <string>
#include <vector>

using std::string; using std::vector;

class SplitTool {
public:
    SplitTool() {}
    virtual vector<string> cut(const string& cnfileContent) = 0;
    virtual ~SplitTool() {}
};


// 用饿汉模式做单例
class SplitToolCppjieba : public SplitTool {
public:
    static SplitToolCppjieba* getInstance();
    vector<string> cut(const string& cnfileContent) override; 
private:
    SplitToolCppjieba();
    static void destroy();
    virtual ~SplitToolCppjieba();

    SplitToolCppjieba(const SplitToolCppjieba& rhs) = delete;
    SplitToolCppjieba& operator=(const SplitToolCppjieba& rhs) = delete;

private:
    static SplitToolCppjieba* _pInstance;
    cppjieba::Jieba* _jieba;
};