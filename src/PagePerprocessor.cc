#include "../include/PagePreprocessor.h"
#include "../include/Configuration.h"
#include "../include/tinyxml2.h"
#include "../include/simhash/simhash/Simhasher.hpp"

#include <iostream>
#include <filesystem>
#include <string>

#include <regex>
#include <vector>
#include <fstream>
#include <sstream>

using std::string;
using std::vector;
using std::ifstream;
using std::ofstream;

using namespace::tinyxml2;
using namespace simhash;
namespace fs = std::filesystem;

// struct RSSItem {
//     string _title;
//     string _link;
//     string _description; 
// };

PagePreprocessor::PagePreprocessor(SplitTool* cuttor) : _cuttor(cuttor) {}

// inline static void readOnePage(vector<RSSItem>& rss, const string& filepath) {

//     ifstream ifs(filepath);
//     if (!ifs.is_open()) {
//         std::cerr << "Fail to open " << filepath << '\n';
//         return;
//     }

//     // 打开xml成功

//     XMLDocument doc;
//     doc.LoadFile(filepath.c_str());
//     if (doc.ErrorID()) {
//         std::cerr << "LoadFile fail" << '\n';
//         return;
//     }

//     XMLElement* itemNode = doc.FirstChildElement("rss")->FirstChildElement("channel")->FirstChildElement("item");

//     while (itemNode) {
//         if (!itemNode->FirstChildElement("description") || !itemNode->FirstChildElement("title") || !itemNode->FirstChildElement("link")) {
//             itemNode = itemNode->NextSiblingElement("item");
//             continue;
//         } 
             

//         string title = itemNode->FirstChildElement("title")->GetText();
//         string link = itemNode->FirstChildElement("link")->GetText();
//         string description = itemNode->FirstChildElement("description")->GetText();
       
//         std::regex reg("<[^>]+>");
//         description = regex_replace(description, reg, "");
//         title = regex_replace(title, reg, "");

//         RSSItem rssItem;

//         rssItem._title = title;
//         rssItem._link = link;
//         rssItem._description = description;

//         rss.push_back(rssItem);

//         itemNode = itemNode->NextSiblingElement("item");
//     }

// }

// inline static void store(const vector<RSSItem>& rss, const string& offsetpath, const string&  pagepath) {
//     ofstream offsetOfs(offsetpath);
//     ofstream  pageOfs(ripepagepath);
//     if (!offsetOfs.is_open() || !ripepageOfs.is_open()) {
//         std::cerr << "Fail to open " << offsetpath << " and " << ripepagepath << '\n';
//         exit(1);
//     }


//     // 都打开成功
//     size_t offset = 0;
//     for (size_t idx = 0; idx != rss.size(); ++idx) {
//         string str = "<doc>\n\t<docid>" + std::to_string(idx + 1) + "</docid>\n\t<title>" + rss[idx]._title 
//                      + "</title>\n\t<link>" + rss[idx]._link + "</link>\n\t<content>" 
//                      + rss[idx]._description + "</content>\n</doc>" + '\n';
//         size_t length = str.size();

//         ripepageOfs << str;
//         offsetOfs << idx + 1 << " " << offset << " " << length << '\n'; 

//         offset += length;
//     }
    
//     std::cout << "Data written to offset.dat and ripepage.dat successfully\n"; 
// }

// void PagePreprocessor::xmlClean() {
    
//     const string cnxmldir = Configuration::getInstance()->getConfigValStr("cnxmldir");

//     if (cnxmldir.size() == 0) {
//         std::cerr << "no cnxml\n";
//         exit(1);
//     }

//     // 遍历每一篇语料，生成RSSItem存入rss里，将rss的每一个Item转换成string，存入offset和ripepage
//     vector<RSSItem> rss;

//     for (const auto& entry : fs::directory_iterator(cnxmldir)) {
//         if (entry.is_regular_file()) {
//             string filepath = entry.path().string();
//             // std::cout << filepath << '\n';
//             readOnePage(rss, filepath);
//         }
//     }
//     _maxid = rss.size();
//     // 全部xml生成RSSItem，存入offsetpath和ripepagepath
//     const string offsetPath = Configuration::getInstance()->getConfigValStr("offsetPath");
//     const string ripepagePath = Configuration::getInstance()->getConfigValStr("ripepagePath");
//     store(rss, offsetPath, ripepagePath);
// }



// inline static void modifyDocid(string& str, const string& newDocid) {
//     size_t start = str.find("<docid>") + 7;
//     size_t end = str.find("</docid>");

//     if (start != string::npos && end != string::npos && start < end) {
//         str.replace(start, end - start, newDocid);
//     } else {
//         std::cout << "docid标签未找到或格式错误" << '\n';
//     }
// }

// void PagePreprocessor::cutRedundantPage() {
//     const string simhash_jieba_dict = Configuration::getInstance()->getConfigValStr("simhash_jieba.dict");
//     const string simhash_hmm_model = Configuration::getInstance()->getConfigValStr("simhash_hmm_model");
//     const string simhash_idf = Configuration::getInstance()->getConfigValStr("simhash_idf");
//     const string simhash_stop_words = Configuration::getInstance()->getConfigValStr("simhash_stop_words");  

//     const string offset = Configuration::getInstance()->getConfigValStr("offsetPath"); 
//     const string ripepage = Configuration::getInstance()->getConfigValStr("ripepagePath");
//     const string nodupoffset = Configuration::getInstance()->getConfigValStr("nodupoffsetPath");
//     const string noduppage = Configuration::getInstance()->getConfigValStr("noduppagePath");

//     ifstream offsetIfs(offset);
//     if (!offsetIfs.is_open()) {
//         std::cerr << "无法打开偏移库文件" << std::endl;
//         exit(1);
//     }

//     ifstream ripePageIfs(ripepage, std::ios::binary);
//     if (!ripePageIfs.is_open()) {
//         std::cerr << "无法打开未去重资源文件" << std::endl;
//         exit(1);
//     }

//     ofstream noDupOffsetOfs(nodupoffset);
//     if (!noDupOffsetOfs.is_open()) {
//         std::cerr << "无法打开去重偏移库文件" << std::endl;
//         exit(1);
//     }

//     ofstream noDupPageOfs(noduppage);
//     if (!noDupPageOfs.is_open()) {
//         std::cerr << "无法打开去重资源文件" << std::endl;
//         exit(1);
//     }
    
//     Simhasher simhasher(simhash_jieba_dict, simhash_hmm_model, simhash_idf, simhash_stop_words);

//     vector<uint64_t> fingerprint(_maxid, 0); // 去重文件的指纹
//     size_t topN = 5; // 取出重要特征词的个数

//     string line = "";
//     //size_t ii = 0;
//     _nodupMaxid = 0; // 用于计算去重后的文章总数
//     while (std::getline(offsetIfs, line)) { // 从偏移库中逐行读取
//         //++ii;
//         int id = 0;
//         size_t offset = 0;
//         size_t length = 0;

//         std::istringstream iss(line);
//         if (!(iss >> id >> offset >> length)) {
//             std::cerr << "解析偏移库文件行错误\n";
//             continue;
//         }

//         // 使用seekg定位到未去重资源文件的偏移位置
//         ripePageIfs.seekg(offset, std::ios::beg);
//         if (!ripePageIfs) {
//             std::cerr << "文件偏移错误，无法跳转到位置: " << offset << '\n';
//             continue;
//         }

//         string buffer(length, '\0');
//         ripePageIfs.read(&buffer[0], length);

//         if (!ripePageIfs) {
//             std::cerr << "读取失败, 文章ID: " << id << std::endl;
//             continue;
//         }

//         string content = extractDescription(buffer);
//         uint64_t curr = 0; // 存当前文章指纹
//         simhasher.make(content, topN, curr);

//         bool isdup = false; // 是否重复
        
//         size_t idx = 0; // fingerprint 下标
//         while (idx != fingerprint.size() && fingerprint[idx] != 0) {
//             if (Simhasher::isEqual(curr, fingerprint[idx]) == true) {
//                 // 重复
//                 isdup = true;
//                 break;
//             }
//             ++idx;
//         }

//         if (isdup) continue; // 重复的话就下一篇文章
        
//         // 不重复

//         // 增加去重文章总数，将指纹放进vector中，然后将索引和文章分别存到已经去重的文件中
//         ++_nodupMaxid;
//         if (fingerprint[idx] == 0) {
//             fingerprint[idx] = curr;
//         }

//         noDupOffsetOfs << _nodupMaxid << " " << offset << " " <<length << '\n';
//         modifyDocid(buffer, std::to_string(_nodupMaxid));
//         noDupPageOfs << buffer;
//     }

//     // 去重结束
//     offsetIfs.close();
//     ripePageIfs.close();
//     noDupPageOfs.close();
//     noDupOffsetOfs.close();

//     //std::cout << ii << '\n';
//     std::cout << "去重成功" << '\n';
// }


inline static size_t processOnePage(const Simhasher& simhasher, const string& filepath, 
                                    ofstream& offsetOfs, ofstream& pageOfs, vector<uint64_t>& fingerprint, size_t topN, size_t offset) {
    ifstream ifs(filepath);
    if (!ifs.is_open()) {
        std::cerr << "Fail to open " << filepath << '\n';
        return offset;
    }

    // 打开xml成功

    XMLDocument doc;
    doc.LoadFile(filepath.c_str());
    if (doc.ErrorID()) {
        std::cerr << "LoadFile fail" << '\n';
        return offset;
    }

    size_t currOffset = offset;
    XMLElement* itemNode = doc.FirstChildElement("rss")->FirstChildElement("channel")->FirstChildElement("item");

    while (itemNode) {
        if (!itemNode->FirstChildElement("description") || !itemNode->FirstChildElement("title") || !itemNode->FirstChildElement("link")) {
            itemNode = itemNode->NextSiblingElement("item");
            continue;
        } 
             
        string title = itemNode->FirstChildElement("title")->GetText();
        string link = itemNode->FirstChildElement("link")->GetText();
        string description = itemNode->FirstChildElement("description")->GetText();
       
        std::regex reg("<[^>]+>");
        description = regex_replace(description, reg, "");
        title = regex_replace(title, reg, "");

        // RSSItem rssItem;

        // rssItem._title = title;
        // rssItem._link = link;
        // rssItem._description = description;

        // 做去重
        uint64_t curr = 0;
        simhasher.make(description, topN, curr);

        bool isdup = false;

        for (const auto& elem : fingerprint) {
            if (Simhasher::isEqual(curr, elem)) {
                isdup = true;
                break;
            }
        }

        // 不重复，写入指纹，偏移和文件流中
        

        if (!isdup) {
            fingerprint.push_back(curr);
            string fileId = to_string(fingerprint.size());
            string str = "<doc>\n\t<docid>" + fileId + "</docid>\n\t<title>" + title 
                     + "</title>\n\t<link>" + link + "</link>\n\t<content>" 
                     + description + "</content>\n</doc>" + '\n';
            size_t length = str.size();

            offsetOfs << fileId << " " << currOffset << " " <<length << '\n';
            pageOfs << str;
            currOffset += length;
        }

        itemNode = itemNode->NextSiblingElement("item");
    }
    //std::cout << filepath << "success process!\n";
    return currOffset;
}

void PagePreprocessor::xmlCleanAndCutRedundantPage() {
    const string cnxmldir = Configuration::getInstance()->getConfigValStr("cnxmldir");
    const string offsetPath = Configuration::getInstance()->getConfigValStr("offsetPath");
    const string pagePath = Configuration::getInstance()->getConfigValStr("pagePath");

    ofstream offsetOfs(offsetPath);
    if (!offsetOfs.is_open()) {
        std::cerr << "Failed to open offset file\n";
        exit(1);
    }

    ofstream pageOfs(pagePath);
    if (!pageOfs.is_open()) {
        std::cerr << "Failed to open page file\n";
        exit(1);
    }

    const string simhash_jieba_dict = Configuration::getInstance()->getConfigValStr("simhash_jieba.dict");
    const string simhash_hmm_model = Configuration::getInstance()->getConfigValStr("simhash_hmm_model");
    const string simhash_idf = Configuration::getInstance()->getConfigValStr("simhash_idf");
    const string simhash_stop_words = Configuration::getInstance()->getConfigValStr("simhash_stop_words");
    
    // size_t idx = 0;
    Simhasher simhasher(simhash_jieba_dict, simhash_hmm_model, simhash_idf, simhash_stop_words);
    vector<uint64_t> fingerprint;
    size_t topN = 5;
    size_t offset = 0;
    for (const auto& entry : fs::directory_iterator(cnxmldir)) {
        if (entry.is_regular_file()) {
            string filepath = entry.path().string();
            // std::cout << filepath << '\n';
            offset = processOnePage(simhasher, filepath, offsetOfs, pageOfs, fingerprint, topN, offset);
        }
    }

    _nodupMaxid = fingerprint.size();
    std::cout << "清理并去重成功！\n";
}

//int TF, int DF, size_t N
// double result = std::log(static_cast<double>(N) / (1 + DF)) * TF;
// return result;

// forDF :  词 词在全部文章中出现次数 词在哪些文章id中出现过
// allTF :  每个下标都是一个文章，string:词 int:该词在当前文章中出现的次数

inline static double TF_IDF(int TF, int DF, size_t N) {
    return std::log(static_cast<double>(N) / (1 + DF)) * TF;
}

inline static void calculateWeight(const unordered_map<string, pair<int, vector<int>>>& forDF, 
                                   const vector<unordered_map<string, int>>& allTF, size_t numOfArticles, unordered_map<string, set<pair<double, int>>>& invertIndex) {
    // w = std::log(static_cast<double>(N) / (1 + DF)) * TF;
    
    // 先处理每一篇文章的词
    for (size_t idx = 0; idx < allTF.size(); ++idx) {
        const unordered_map<string, int>& page = allTF[idx];
        vector<pair<string, double>> nonNormalWight(page.size());
        size_t wordIdx = 0; // 记录下标
        double sumOfSquares = 0.0;
        for (const auto& pair : page) {
            const string& word = pair.first;
            int TF = pair.second;
            double weight = TF_IDF(TF, forDF.at(word).first, numOfArticles);
            nonNormalWight[wordIdx] = {word, weight};

            sumOfSquares += weight * weight;
            ++wordIdx;
        }

        // 处理完此文章中的每一个词，词和其w保存在nonNormalWeight，词归一化分母为sumOfSquares
        for (const pair<string, double>& pair: nonNormalWight) {
            const string& word = pair.first;
            double weight = pair.second;
            // 归一化
            weight /= std::sqrt(sumOfSquares);

            // 写入invertIndex
            invertIndex[word].insert({weight, idx + 1});
        } 
    }
}

inline static string extractDescription(const string& content) {
    string start_tag = "<content>";
    string end_tag = "</content>";

    size_t start_pos = content.find(start_tag);
    if (start_pos == std::string::npos) {
        return "";  // 找不到标签
    }

    start_pos += start_tag.length();  // 跳过起始标签
    size_t end_pos = content.find(end_tag, start_pos);
    if (end_pos == std::string::npos) {
        return "";  // 找不到结束标签
    }

    // 提取标签中的内容
    return content.substr(start_pos, end_pos - start_pos);
}

inline bool isChineseWord(const string& word) {
    std::regex chinese_regex(u8"^[\u4e00-\u9fa5]+$");
    return std::regex_match(word, chinese_regex);
} 

void PagePreprocessor::buildInvertIndexMap() {
    // 词 权重 文章id
    unordered_map<string, set<pair<double, int>>> invertIndex;
    //size_t _nodupMaxid = 3649;

    const string offset = Configuration::getInstance()->getConfigValStr("offsetPath");
    const string page = Configuration::getInstance()->getConfigValStr("pagePath");
    const string stop_words_zh = Configuration::getInstance()->getConfigValStr("stop_words_zh");
    unordered_set<string> stopwords;

    ifstream OffsetIfs(offset);
    if (!OffsetIfs.is_open()) {
        std::cerr << "Failed to open offset file\n";
        exit(1);
    }

    ifstream PageIfs(page);
    if (!PageIfs.is_open()) {
        std::cerr << "Failed to open page file\n";
        exit(1);
    }

    // 打开停用词文件
    ifstream stopWordIfs(stop_words_zh);
    if (!stopWordIfs.is_open()) {
        std::cerr << "no stop_words_zh\n";
    }

    // 打开停用词文件成功
    string stopWordLine;
    while (std::getline(stopWordIfs,stopWordLine)) {
        // 每行只有一个词
        stopwords.insert(stopWordLine);
    }

    stopWordIfs.close(); // 停用词库生成成功

    // 词 词在全部文章中出现次数 词在哪些文章id中出现过
    unordered_map<string, pair<int, vector<int>>> forDF; 
    
    // 每个下标都是一个文章，string:词 int:该词在当前文章中出现的次数
    vector<unordered_map<string, int>> allTF(_nodupMaxid);

    string line = "";
    size_t idx = 0; // 用于保存下标，文章的篇数
    while (std::getline(OffsetIfs, line)) {
        int id = 0;
        size_t offset = 0;
        size_t length = 0;
        unordered_map<string, int> forTF;

        std::istringstream iss(line);
        if (!(iss >> id >> offset >> length)) {
            std::cerr << "解析偏移库文件行错误\n";
            continue;
        }

        // if (idx > 3565) {
        //     std::cout << "当前id: " << id <<"尝试跳转到偏移: " << offset << " 长度: " << length << '\n';
        // }
        // 使用seekg定位到未去重资源文件的偏移位置
        PageIfs.seekg(offset, std::ios::beg);
        if (!PageIfs) {
            std::cerr << "文件偏移错误，无法跳转到位置: " << offset << '\n';
            continue;
        }

        // if (idx > 3570 && idx < 3585) {
        //     // 获取当前位置
        //     size_t currentPosition = noDupPageIfs.tellg();

        //     // 跳转到文件末尾
        //     noDupPageIfs.seekg(0, std::ios::end);
        //     size_t endPosition = noDupPageIfs.tellg();

        //     // 计算当前位置到文件末尾的距离
        //     size_t remainingLength = endPosition - currentPosition;
            
        //     std::cout << "从当前位置到文件末尾的距离: " << remainingLength << " 字节" << std::endl;
        //     std::cout << length << '\n';

        //     // 跳回原来的位置以继续处理文件
        //     noDupPageIfs.seekg(currentPosition, std::ios::beg);
        // }

        string buffer(length + 1, '\0');
        PageIfs.read(&buffer[0], length);

        if (!PageIfs) {
            std::cerr << "读取失败, 文章ID: " << id << std::endl;
            continue;
            //exit(1);
        }

        // 文章可能是中英文混合的，纯中文或纯英文，这里可以清洗
        
        // 此处文章都是纯中文，故直接用cppjieba
        // 将其中的content提取出来
        string content = extractDescription(buffer);
        vector<string> cutedWords = _cuttor->cut(content); 
        for (const auto& word : cutedWords) {
            if (stopwords.find(word) == stopwords.end()) {
                ++forTF[word];
            }
        }


        // 得到了该篇文章的所有词的出现次数
        allTF[idx] = forTF;
        
        for (const pair<string, int>& TFpair : forTF) {
            pair<int, vector<int>>& DFpair = forDF[TFpair.first];
            DFpair.first += TFpair.second;
            DFpair.second.push_back(idx + 1); // 文章id
        }

        ++idx; 
    }

    if (_nodupMaxid != idx) std::cout << "去重和倒排中文章总数不一致\n"; // 这里到前面都没问题

    // 处理完所有文章，将allTF和forDF交给caculateWight算
    calculateWeight(forDF, allTF, _nodupMaxid, invertIndex);

    // 处理完，写入倒排索引文件、
    const string& invertindexPath = Configuration::getInstance()->getConfigValStr("invertindex");
    ofstream invertIndexOfs(invertindexPath);
    if (!invertIndexOfs.is_open()) {
        std::cerr << "倒排索引文件打开失败\n";
        exit(1);
    }

    // 打开成功
    // unordered_map<string, set<pair<double, int>>> invertIndex;
    for (const auto& elem : invertIndex) {
        const string& word = elem.first;
        set<pair<double, int>> set = elem.second;
        invertIndexOfs << word << " ";
        for (const pair<double, int>& pair : set) {
            invertIndexOfs << pair.second << " " << pair.first << " ";
        }
        invertIndexOfs << '\n';
    }
    invertIndexOfs.close();
    std::cout << "倒排索引写入倒排索引文件成功！\n";
}

