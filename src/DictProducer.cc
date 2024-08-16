#include "../include/DictProducer.h"
#include "../include/SplitTool.h"

#include <fstream>
#include <sstream>
#include <map>
DictProducer::DictProducer(const string& dir) {

} 

DictProducer::DictProducer(const string& dir, SplitTool* cuttor) {

} 

inline static bool isPunctuation(char c) {
    return ispunct(c) || c == '\r'; // 判断是否为标点符号或 \r
}

void DictProducer::buildEndict() {
    using std::ifstream; using std::map; using std::stringstream;

    // 没有英文语料
    if (_enfiles.size() == 0) return;

    // 遍历enfiles，读取enfiles路径，然后读取路径文件信息，写入map<string, int>里，最后写入_dict
    map<string, int> dictmap;
    for (const string& enfilepath : _enfiles) {
        // 打开文件
        ifstream ifs(enfilepath);
        if (ifs.is_open()) {
            std::cerr << "Failed to open file!" << '\n';
            // log() to zsh and .log
        }

        string word, line;

        
        while (std::getline(ifs, line)) {
            // 数据清洗
            for (char& c : line) {
                if (isupper(c)) c = tolower(c);

                if (isPunctuation(c)) c = ' ';
            }

            // 去重读入
            stringstream ss(line);
            while (ss >> word) {
                ++dictmap[word];
            }
        }

    }

    // dictmap 写入 _dict
    for (const pair<string, int>& pair : dictmap) {
        _dict.emplace_back(pair.first, pair.second);
    }

}

void DictProducer::buildCndict() {

}

void DictProducer::storeDict(const char* filepa) { // 将词典写入文件 

}
    
void DictProducer::showFiles() const { // 查看文件路径，作为测试用

}
void DictProducer::showDict() const { // 查看词典，作为测试用

}
void DictProducer::getFiles() { // 获取文件的绝对路径

}
void DictProducer::pushDict(const string& word) { // 存储某个单词 

}

