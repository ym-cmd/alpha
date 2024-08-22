#include "../include/DictProducer.h"
#include "../include/SplitTool.h"
#include "../include/Configuration.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>

using std::ifstream; 
using std::ofstream;
using std::stringstream; 
using std::unordered_map; 
using std::unordered_set;

namespace fs = std::filesystem;

DictProducer::DictProducer() : _cuttor(nullptr) {} 

DictProducer::DictProducer(SplitTool* cuttor) : _cuttor(cuttor) {} 

inline static bool isPunctuation(char c) {
    return ispunct(c) || c == '\r' || c == '\n' || isdigit(c); // 判断是否为标点符号或 \r 或 \n 或数字
}

void DictProducer::buildEndict() {
    // 从Configuration里面读取英文文件路径
    const string enfiledir = Configuration::getInstance()->getConfigValStr("enfiledir");

    // 没有英文语料
    if (enfiledir.size() == 0) {
        std::cerr << "no english corpus\n";
        exit(1);
    }

    // 遍历enfiledir，读取enfiles路径，然后读取路径文件信息，清洗掉换行和标点，再过滤停用词
    // 写入unordered_map<string, int>里，最后生成英文词典

    
    // 将停用词全部读取到unordered_set里，后面用于过滤
    unordered_map<string, int> dictmap;
    const string stop_words_eng = Configuration::getInstance()->getConfigValStr("stop_words_eng");
    unordered_set<string> stopwords;

    // 打开停用词文件
    ifstream ifs(stop_words_eng);
    if (!ifs.is_open()) {
        std::cerr << "no stop_words_eng\n";
    }

    // 打开停用词文件成功
    string line;
    while (std::getline(ifs,line)) {
        // 每行只有一个词
        stopwords.insert(line);
    }

    ifs.close();

    // 使用c++17的filesystem, 循环打开dir,将文件清洗并读入dictmap
    for (const auto& entry : fs::directory_iterator(enfiledir)) {
        if (entry.is_regular_file()) {
            string filepath = entry.path().string();
            
            ifstream ifs(filepath);
            if (!ifs.is_open()) {
                std::cerr << "Failed to open " << filepath << "!" << '\n';
                // log() to zsh and .log
                continue; // 下一个文件
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
                    // 判断单词是不是停用词, 不是停用词就加进去
                    if (stopwords.find(word) == stopwords.end()) {
                        ++dictmap[word];
                    }
                }
            }
            ifs.close();
        }
    }

    // 生成英文词典
    const string endictpath = Configuration::getInstance()->getConfigValStr("endictpath");
    ofstream ofs(endictpath);
    if (!ofs.is_open()) {
        std::cerr << "Failed to open endict for writing." << '\n';
        exit(1);
    }

    // 打开endict成功，写入
    for (const auto& pair : dictmap) {
        ofs << pair.first << " " << pair.second << '\n';
    }

    ofs.close();

    std::cout << "Data written to endict successfully." << '\n';
}

// 清洗中文语料并假定中文用utf8，3字节（删除\r,\n)
inline static void cleanOut(string& str) {
    int len = str.size();
    int j = 0;

    for (int i = 0; i < len;) {
        if (str[i] == '\n' || str[i] == '\r' || (str[i] & 0x80) == 0) {
            // 如果是换行符或英文，跳过
            ++i;
        } else {
            // 如果是中文字符 (UTF-8)，假设是三字节编码，复制三个字节
            if (i + 2 < len) {
                str[j++] = str[i];
                str[j++] = str[i + 1];
                str[j++] = str[i + 2];
            }
            i += 3; // 移动到下一个字符（中文字符是3字节的）
        }
    }

    str.resize(j);  // 调整字符串大小，删除多余部分
}

// 打开中文文件目录，一篇一篇进行cppjieba.cut
// 对返回的清洗过的词进行过滤停用词，放入unordered_map里
// 再放入_dict里
void DictProducer::buildCndict() {

    // 获取中文语料目录
    const string cnfiledir = Configuration::getInstance()->getConfigValStr("cnfiledir");

    // 没有中文语料目录，退出
    if (cnfiledir.size() == 0) {
        std::cerr << "no Chinese corpus\n";
        exit(1);
    }

    // 将停用词全部读取到unordered_set里，后面用于过滤
    unordered_map<string, int> dictmap;
    const string stop_words_zh = Configuration::getInstance()->getConfigValStr("stop_words_zh");
    unordered_set<string> stopwords;

    // 打开停用词文件
    ifstream ifs(stop_words_zh);
    if (!ifs.is_open()) {
        std::cerr << "no stop_words_zh\n";
    }

    // 打开停用词文件成功
    string line;
    while (std::getline(ifs,line)) {
        // 每行只有一个词
        stopwords.insert(line);
    }

    ifs.close();

    // 有中文语料目录
    for (const auto& entry : fs::directory_iterator(cnfiledir)) {
        if (entry.is_regular_file()) {
            string filepath = entry.path().string();

            ifstream ifs(filepath, std::ios::in | std::ios::binary);
            if (!ifs.is_open()) {
                std::cerr << "Failed to open " << filepath << "!" << '\n';
                // log() to zsh and .log
                continue; // 下一个文件
            }

            // 读取文件内容到 string 中
            string cnfileContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

            // 检查是否发生读取错误
            if (ifs.fail()) {
                std::cerr << "文件读取失败: " << filepath << std::endl;
                ifs.close();
                continue;  // 跳过当前文件，继续处理下一个文件
            }
            
            // 已经成功写入string，关闭文件描述符
            ifs.close();

            // 清洗掉换行符，英文词
            cleanOut(cnfileContent);

            vector<string> cleanedWords = _cuttor->cut(cnfileContent);

            for (const auto& word : cleanedWords) {
                // 过滤停用词
                if (!stopwords.count(word)) {
                    ++dictmap[word];
                }
            }
        }
    }

    // 成功生成中文词典，放入_cndict
    for (const auto& pair : dictmap) {
        _cndict.emplace_back(pair.first, pair.second);
    }
}

void DictProducer::storeCnDict() { // 将中文词典写入文件
    const string filepa = Configuration::getInstance()->getConfigValStr("cndictpath");
    if (filepa.size() == 0) {
        std::cerr << filepa << '\n';
        return;
    }

    ofstream ofs(filepa);
    if (!ofs.is_open()) {
        std::cerr << "Failed to open cndict to storeCndict\n"; 
        return;
    }

    for (const auto& pair : _cndict) {
        ofs << pair.first << " " << pair.second << '\n';
    }

    ofs.close();

    std::cout << "Data written to cndict successfully." << '\n';
}


static size_t getByteNum_UTF8(const char byte) {
    int byteNum = 0;
    for (size_t i = 0; i < 6; ++i) {
        if (byte & (1 << (7 - i))) 
            ++byteNum;
        else 
            break;
    }

    return byteNum == 0 ? 1 : byteNum;
}

void DictProducer::buildCnDictIndex() {
    //  vector<pair<string, int>> >> unordered_map<string, unordered_set<int>>

    int i = 0; // 记录下标
    for (const auto& elem : _cndict) {
        string word = elem.first;
        size_t charNums = word.size()/getByteNum_UTF8(word[0]);
        for (size_t idx = 0, n = 0; n != charNums; ++idx, ++n) {
            size_t charlen = getByteNum_UTF8(word[idx]);
            string subword = word.substr(idx, charlen);

            _cnindex[subword].insert(i);
            idx += (charlen - 1);
        }

        ++i;
    }
}


// unordered_map<string, unordered_set<int>> >> .dat
void DictProducer::storeCnDictIndex() {
    const string& cndictIndexPath = Configuration::getInstance()->getConfigValStr("cndictIndexPath");
    ofstream ofs(cndictIndexPath);
    if (!ofs.is_open()) {
        std::cerr << "Failed to open cndictIndexPath " << cndictIndexPath << '\n';
        return;
    }

    for (const auto& pair : _cnindex) {
        ofs << pair.first << " ";
        for (const auto& elem : pair.second) {
            ofs << elem << " ";
        }
        ofs << '\n';
    }

    ofs.close();

    std::cout << "Data written to cnindex successfully\n";    
}

void DictProducer::showFiles() const { // 查看文件路径，作为测试用

}
void DictProducer::showDict() const { // 查看词典，作为测试用

}

