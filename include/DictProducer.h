#include <vector>
#include <string>
#include <set>
#include <iostream>
#include <map>
using std::vector; using std::string; using std::pair; using std::set; using std::map;

class SplitTool;

class DictProducer {  
public:
    DictProducer(const string& dir);
    DictProducer(const string& dir, SplitTool* cuttor); // 专为中文处理
    void buildEndict(); // 创建英文词典
    void buildCndict(); // 创建中文词典
    void storeDict(const char* filepa); // 将词典写入文件
    
    void showFiles() const; // 查看文件路径，作为测试用
    void showDict() const; // 查看词典，作为测试用

    void getFiles(); // 获取文件的绝对路径
    void pushDict(const string& word); // 存储某个单词 

private:
    vector<string> _cnfiles; // 语料库中文文件的绝对路径集合
    vector<string> _enfiles; // 语料库英文文件的绝对路径集合
    vector<pair<string, int>> _dict; // 中英文词典
    map<string, set<int>> _index; // 词典索引
    SplitTool* _cuttor; // 分词工具
};