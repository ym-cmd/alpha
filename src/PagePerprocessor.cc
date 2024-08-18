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

struct RSSItem {
    string _title;
    string _link;
    string _description; 
};

inline static void readOnePage(vector<RSSItem>& rss, const string& filepath) {

    ifstream ifs(filepath);
    if (!ifs.is_open()) {
        std::cerr << "Fail to open " << filepath << '\n';
        return;
    }

    // 打开xml成功

    XMLDocument doc;
    doc.LoadFile(filepath.c_str());
    if (doc.ErrorID()) {
        std::cerr << "LoadFile fail" << '\n';
        return;
    }

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

        RSSItem rssItem;

        rssItem._title = title;
        rssItem._link = link;
        rssItem._description = description;

        rss.push_back(rssItem);

        itemNode = itemNode->NextSiblingElement("item");
    }

}

inline static void store(const vector<RSSItem>& rss, const string& offsetpath, const string& ripepagepath) {
    ofstream offsetOfs(offsetpath);
    ofstream ripepageOfs(ripepagepath);
    if (!offsetOfs.is_open() || !ripepageOfs.is_open()) {
        std::cerr << "Fail to open " << offsetpath << " and " << ripepagepath << '\n';
        exit(1);
    }


    // 都打开成功
    size_t offset = 0;
    for (size_t idx = 0; idx != rss.size(); ++idx) {
        string str = "<doc>\n\t<docid>" + std::to_string(idx + 1) + "</docid>\n\t<title>" + rss[idx]._title 
                     + "</title>\n\t<link>" + rss[idx]._link + "</link>\n\t<content>" 
                     + rss[idx]._description + "</content>\n</doc>" + '\n';
        size_t length = str.size();

        ripepageOfs << str;
        offsetOfs << idx + 1 << " " << offset << " " << length << '\n'; 

        offset += length;
    }
    
    std::cout << "Data written to offset.dat and ripepage.dat successfully\n"; 
}

void PagePreprocessor::xmlClean() {
    
    const string cnxmldir = Configuration::getInstance()->getConfigValStr("cnxmldir");

    if (cnxmldir.size() == 0) {
        std::cerr << "no cnxml\n";
        exit(1);
    }

    // 遍历每一篇语料，生成RSSItem存入rss里，将rss的每一个Item转换成string，存入offset和ripepage
    vector<RSSItem> rss;

    for (const auto& entry : fs::directory_iterator(cnxmldir)) {
        if (entry.is_regular_file()) {
            string filepath = entry.path().string();
            // std::cout << filepath << '\n';
            readOnePage(rss, filepath);
        }
    }
    _maxid = rss.size();
    // 全部xml生成RSSItem，存入offsetpath和ripepagepath
    const string offsetPath = Configuration::getInstance()->getConfigValStr("offsetPath");
    const string ripepagePath = Configuration::getInstance()->getConfigValStr("ripepagePath");
    store(rss, offsetPath, ripepagePath);
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

void PagePreprocessor::cutRedundantPage() {
    const string simhash_jieba_dict = Configuration::getInstance()->getConfigValStr("simhash_jieba.dict");
    const string simhash_hmm_model = Configuration::getInstance()->getConfigValStr("simhash_hmm_model");
    const string simhash_idf = Configuration::getInstance()->getConfigValStr("simhash_idf");
    const string simhash_stop_words = Configuration::getInstance()->getConfigValStr("simhash_stop_words");  

    const string offset = Configuration::getInstance()->getConfigValStr("offsetPath"); 
    const string ripepage = Configuration::getInstance()->getConfigValStr("ripepagePath");
    const string nodupoffset = Configuration::getInstance()->getConfigValStr("nodupoffsetPath");
    const string noduppage = Configuration::getInstance()->getConfigValStr("noduppagePath");

    ifstream offsetIfs(offset);
    if (!offsetIfs.is_open()) {
        std::cerr << "无法打开偏移库文件" << std::endl;
        exit(1);
    }

    ifstream ripePageIfs(ripepage, std::ios::binary);
    if (!ripePageIfs.is_open()) {
        std::cerr << "无法打开未去重资源文件" << std::endl;
        exit(1);
    }

    ofstream noDupOffsetOfs(nodupoffset);
    if (!noDupOffsetOfs.is_open()) {
        std::cerr << "无法打开去重偏移库文件" << std::endl;
        exit(1);
    }

    ofstream noDupPageOfs(noduppage);
    if (!noDupPageOfs.is_open()) {
        std::cerr << "无法打开去重资源文件" << std::endl;
        exit(1);
    }
    
    Simhasher simhasher(simhash_jieba_dict, simhash_hmm_model, simhash_idf, simhash_stop_words);

    vector<uint64_t> fingerprint(_maxid, 0); // 去重文件的指纹
    size_t topN = 5; // 取出重要特征词的个数

    string line = "";
    //size_t ii = 0;
    _nodupMaxid = 0; // 用于倒排中的文档总数
    while (std::getline(offsetIfs, line)) { // 从偏移库中逐行读取
        //++ii;
        int id = 0;
        size_t offset = 0;
        size_t length = 0;

        std::istringstream iss(line);
        if (!(iss >> id >> offset >> length)) {
            std::cerr << "解析偏移库文件行错误\n";
            continue;
        }

        // 使用seekg定位到未去重资源文件的偏移位置
        ripePageIfs.seekg(static_cast<std::streampos>(offset), std::ios::beg);
        if (!ripePageIfs) {
            std::cerr << "文件偏移错误，无法跳转到位置: " << offset << '\n';
            continue;
        }

        string buffer(length, '\0');
        ripePageIfs.read(&buffer[0], length);

        if (!ripePageIfs) {
            std::cerr << "读取失败, 文章ID: " << id << std::endl;
            continue;
        }

        string content = extractDescription(buffer);
        uint64_t curr = 0; // 存当前文章指纹
        simhasher.make(content, topN, curr);

        bool isdup = false; // 是否重复
        
        size_t idx = 0; // fingerprint 下标
        while (idx != fingerprint.size() && fingerprint[idx] != 0) {
            if (Simhasher::isEqual(curr, fingerprint[idx]) == true) {
                // 重复
                isdup = true;
                break;
            }
            ++idx;
        }

        if (isdup) continue; // 重复的话就下一篇文章
        
        // 不重复

        // 将指纹放进vector中，然后将索引和文章分别存到已经去重的文件中
        if (fingerprint[idx] == 0) {
            fingerprint[idx] = curr;
        }

        ++_nodupMaxid;

        noDupOffsetOfs << _nodupMaxid << " " << offset << " " <<length << '\n';
        noDupPageOfs << buffer;
    }

    // 去重结束
    offsetIfs.close();
    ripePageIfs.close();
    noDupPageOfs.close();
    noDupOffsetOfs.close();

    //std::cout << ii << '\n';
    std::cout << "去重成功" << '\n';
}

void PagePreprocessor::buildInvertIndexMap() {
    unordered_map<string, set<pair<double, int>>> invertIndex;

    unordered_map<string, int> forDF; // 词 词在全部文章中出现次数
    unordered_map<string, vector<int>> stridx; // 词 该词在那些文章id中出现过

    // 每个下标都是一个文章，string:词 int:该词在当前文章中出现的次数
    vector<unordered_map<string, int>> allTF;

    //for ()






}

