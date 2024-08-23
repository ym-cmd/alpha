#pragma once

#include <vector>
#include <string>
#include <unordered_set>
#include <iostream>

#include <unordered_map>
using std::vector; 
using std::string; 
using std::pair; 
using std::unordered_set; 
using std::unordered_map;

class SplitTool;

// 中文用编辑距离，需要词典 + 词典索引
// 英文用trie树，需要词典，不需要词典索引
class DictProducer {  
public:
    DictProducer();
    DictProducer(SplitTool* cuttor); // 专为中文处理
    void buildEndict(); // 创建英文词典
    void buildCndict(); // 创建中文词典
    void storeCnDict(); // 将中文词典写入文件
    

    // 测试

    // 将中文词典生成词典索引
    void buildCnDictIndex();

    // 将索引生成中文词典索引文件
    void storeCnDictIndex();
    
    void showFiles() const; // 查看文件路径，作为测试用
    void showDict() const; // 查看词典，作为测试用
//private:
    // string enfiledir = ""; // 语料库英文文件的文件夹路径
    // string cnfiledir = ""; // 语料库中文文件的文件夹路径 

    // vector<string> _cnfiles; // 语料库中文文件的绝对路径集合
    // vector<string> _enfiles; // 语料库英文文件的绝对路径集合

    vector<pair<string, int>> _cndict; // 中文词典
    unordered_map<string, unordered_set<int>> _cnindex; // 中文词典索引
    SplitTool* _cuttor; // 分词工具(其实为了高拓展性，本来可以直接用单例的)
};