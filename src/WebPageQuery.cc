#include "../include/WebPageQuery.h"
#include "../include/Configuration.h"
#include "../include/SplitTool.h"
#include "../include/Mylogger.h"
#include "../include/utils.h"
#include "../include/nlohmann/json.hpp"

#include <fstream>
#include <sstream>
#include <vector>
#include <complex>
#include <set>
#include <queue>

using std::set;
using std::pair;
using std::vector;
using std::string;
using std::ifstream;
using std::istringstream;
using std::unordered_map;
using std::priority_queue;

WebPageQuery* WebPageQuery::_pInstance = nullptr;
pthread_once_t WebPageQuery::_once = PTHREAD_ONCE_INIT;

WebPageQuery* WebPageQuery::getInstance() {
    pthread_once(&_once, init_r);
    return _pInstance;
}

void WebPageQuery::generateBaseVectors(vector<double>& base, const unordered_map<string, int>& afterCleanWords) {
    size_t idx = 0;
    // 由于此处将查询词设为一篇文章，TF_IDF算法时则IDF为定值
    double IDF = std::log(3/2);
    double sumOfSquares = 0.0;
    for (const auto& pair : afterCleanWords) {
        double weight = IDF * pair.second;
        base[0] = weight;

        sumOfSquares += weight;
    }

    // 归一化
    for (auto& weight : base) {
        weight /= std::sqrt(sumOfSquares);
    }
}

vector<int> findCommonIDs(const vector<set<pair<int, double>>>& sets) {
    if (sets.empty()) return {};

    set<int> commonIDs;
    for (const auto& pair : sets[0]) {
        commonIDs.insert(pair.first);
    }

    for (size_t i = 1; i < sets.size(); ++i) {
        set<int> currIDs;
        for (const auto& pair : sets[i]) {
            currIDs.insert(pair.first);
        }

        set<int> temp;
        std::set_intersection(commonIDs.begin(), commonIDs.end(),
                              currIDs.begin(), currIDs.end(),
                              std::inserter(temp, temp.begin()));
        commonIDs = std::move(temp);
    }
}

// 计算两个向量的余弦相似度
double countCosineSimilarity(const std::vector<double>& A, const std::vector<double>& B) {
    double dotProduct = 0.0, normA = 0.0, normB = 0.0;
    for (size_t i = 0; i < A.size(); ++i) {
        dotProduct += A[i] * B[i];
        normA += A[i] * A[i];
        normB += B[i] * B[i];
    }
    return dotProduct / (std::sqrt(normA) * std::sqrt(normB));
}

string WebPageQuery::doQuery(string keys) {
    const string& offsetIndexFile = Configuration::getInstance()->getConfigValStr("offsetIndex");
    ifstream offsetIfs(offsetIndexFile);
    if (!offsetIfs.is_open()) {
        LogError("偏移库文件打开错误");
        exit(1);
    }

    // 分词
    vector<string> afterCutWords = SplitToolCppjieba::getInstance()->cut(keys);
    
    // 过滤停用词（目前只是中文）
    // 词，TF (查询词)
    unordered_map<string, int> afterCleanWords = utils::getInstance()->filterStopWords(afterCutWords);

    // 用TF-IDF得到每个关键词的权重系数，变成基准向量base0, 然后再归一化变成基准向量base
    vector<double> base(afterCleanWords.size());
    
    // 生成基准向量
    generateBaseVectors(base, afterCleanWords);

    // 找到相同的网页id
    vector<set<pair<int, double>>> wordsSet;
    for (const auto& pair : afterCleanWords) {
        if (_invertIndex.find(pair.first) != _invertIndex.end()) {
            wordsSet.emplace_back(_invertIndex[pair.first]);
        }   
    }

    vector<int> commonIDs = findCommonIDs(wordsSet);
    if (commonIDs.size() == 0) {
        return returnNoAnswer();
    }


    // 计算每个网页的向量和基准向量的余弦相似度，并存入优先队列
    using DocWeightPair = std::pair<double, int>; // 对应的相似度 文章id，根据相似度来排序
    priority_queue<DocWeightPair, vector<DocWeightPair>, std::greater<DocWeightPair>> pq;

    for (int docId : commonIDs) {
        vector<double> docVector(afterCleanWords.size());
        int i = 0;
        for (const auto& pair : afterCleanWords) {
            auto& wordSet = _invertIndex[pair.first];
            auto it = std::find_if(wordSet.begin(), wordSet.end(), [&](const std::pair<int, double>& p){
                return p.first == docId;
            });

            if (it != wordSet.end()) {
                docVector[i] = it->second;
            }
            ++i;
        }
        
        // 计算余弦相似度
        double cosineSimilarity = countCosineSimilarity(base, docVector);

        pq.push({cosineSimilarity, docId});
        if (pq.size() > 10) {
            pq.pop();
        }
    } // 优先队列保存了余弦相似度最大的10个文章id

    // 取出相应的文章id
    vector<int> topDocIDs;
    while (!pq.empty()) {
        topDocIDs.push_back(pq.top().second);
        pq.pop();
    }

    // 根据对应的文章id取出文章
    vector<string> docs(topDocIDs.size());
    int idx = 0;
    for (const auto& val : topDocIDs) {
        // 移动到文章id对应的offset
        offsetIfs.seekg(_offsetIndex[val].first, std::ios::beg);

        // 读取对应长度的string放到docs里
        string buffer(_offsetIndex[val].second, '\0');
        offsetIfs.read(&buffer[0], _offsetIndex[val].second);
        docs[idx] = buffer;

        // 处理下一篇文章
        ++idx; 
    }

    // 生成json并返回
    return createJson(topDocIDs, docs);
}

// 需要解析string中的title,link, content等
string WebPageQuery::createJson(const vector<int>& topDocIDs, const vector<string>& docs) {
    nlohmann::json root;
    nlohmann::json arr = nlohmann::json::array();
    for (size_t i = 0; i < topDocIDs.size(); ++i) {
        if (i >= docs.size()) continue;
        
        // 解析字符串
        string title = utils::getInstance()->extractTagContent(docs[i], "title");
        string link = utils::getInstance()->extractTagContent(docs[i], "link");
        string content = utils::getInstance()->extractTagContent(docs[i], "content");

        nlohmann::json elem;
        elem["title"] = title;
        elem["url"] = link;
        elem["content"] = content;

        arr.push_back(elem);
    }
    root["files"] = arr;
    return root.dump();
}

string WebPageQuery::returnNoAnswer() {
    nlohmann::json root;
    nlohmann::json arr;
    nlohmann::json elem;

    elem["title"] = "404, not found";
    elem["summary"] = "未找到你搜索的内容";
    elem["url"] = "";
    
    arr.push_back(elem);

    root["files"] = arr;
    return root.dump();
}

WebPageQuery::WebPageQuery() {
    loadInvertIndex();
    loadOffsetIndex();
}

void WebPageQuery::loadInvertIndex() {
    const string invertIndexPath = Configuration::getInstance()->getConfigValStr("invertIndex");
    ifstream ifs(invertIndexPath);
    if (!ifs.is_open()) {
        LogError("Failed to open invertIndex File\n");
        exit(1);
    }

    string line;
    while (std::getline(ifs, line)) {
        string word, docIdStr, weightStr;
        istringstream iss(line);
        iss >> word;
        while (iss >> docIdStr >> weightStr) {
            int docId = std::stoi(docIdStr);
            double weight = std::stod(weightStr);

            _invertIndex[word].insert(std::make_pair(docId, weight));
        }
    }
    ifs.close();
    LogInfo("loadInvertIndex success\n");
}

void WebPageQuery::loadOffsetIndex() {
    const string invertIndexPath = Configuration::getInstance()->getConfigValStr("offsetIndex");
    ifstream ifs(invertIndexPath);
    if (!ifs.is_open()) {
        LogError("Failed to open invertIndex File\n");
        exit(1);
    }

    string line;
    while (std::getline(ifs, line)) {
        string posStr, lengthStr;
        istringstream iss(line);
        while (iss >> posStr >> lengthStr) {
            int pos = stoi(posStr);
            int length = stoi(lengthStr);
            _offsetIndex.emplace_back(pos, length);
        }
    }
    ifs.close();
    LogInfo("loadOffsetIndex success\n");

}

void WebPageQuery::destroy() {
    if (_pInstance) {
        delete _pInstance;
        _pInstance = nullptr;
    }
    // std::cout << "WebPageQuery destroy\n";
}

// init_r在pthread_once, 只会被执行一次
void WebPageQuery::init_r() {
    _pInstance = new WebPageQuery();
    atexit(destroy); // 注册destroy()，结束后自动调用进行析构
}




