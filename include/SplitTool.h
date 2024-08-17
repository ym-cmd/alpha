#pragma once

#include <string>
#include <vector>
using std::string; using std::vector;

class SplitTool {
public:
    SplitTool();
    virtual vector<string> cut() = 0;
    virtual ~SplitTool();
};


// 从Configuration中读取语料，清洗后保存为vector<string>返回
class Configuration;
class SplitToolCppjieba : public SplitTool {
public:
    SplitToolCppjieba(Configuration conf);
    vector<string> cut() override; 
    virtual ~SplitToolCppjieba();
private:
    Configuration& _conf; 
};