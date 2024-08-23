#include "../include/KeyRecommander.h"
#include "../include/Configuration.h"
#include "../include/utils.h"
#include "../include/Mylogger.h"
#include "../include/nlohmann/json.hpp"
#include "../include/SplitTool.h"

#include <fstream>
#include <sstream>
#include <unordered_set>
#include <queue>


using std::priority_queue;
using std::unordered_map;
using std::unordered_set;
using std::istringstream;
using std::ifstream;
using std::string;
using std::vector;
using std::pair;


KeyRecommander* KeyRecommander::_pInstance = KeyRecommander::getInstance();

KeyRecommander* KeyRecommander::getInstance() {
    if (_pInstance == nullptr) {
        atexit(destroy);
        _pInstance = new KeyRecommander();
    }
    return _pInstance;
}

bool KeyRecommander::compare(const std::tuple<std::string, int, int>& a, const std::tuple<std::string, int, int>& b) {
    // 先比较第三个元素，小的优先级更高
    if (std::get<2>(a) != std::get<2>(b)) {
        return std::get<2>(a) > std::get<2>(b); // 注意这里用 '>' 因为我们想要小的优先级更高
    }
    // 如果第三个元素相等，则比较第二个元素，大的优先级更高
    if (std::get<1>(a) != std::get<1>(b)) {
        return std::get<1>(a) < std::get<1>(b); // 注意这里用 '<' 因为我们想要大的优先级更高
    }
    // 如果前两个元素都相等，则按照字符串的字典顺序来排序
    return std::get<0>(a) > std::get<0>(b); // 字典序比较
}

std::string KeyRecommander::doQuery(const std::string& keys) {
    if (keys.size() == 0) return returnNoAnswer();
    // 判断中文还是英文， 英文trie树，中文编辑距离
    if (utils::getInstance()->isChinese(keys)) {
        // 中文
        // 将查询词分成一个一个字
        unordered_set<string> character;
        unordered_map<string, int> candidateKeys;
        for (size_t i = 0; i < keys.size(); i += 3) {
            character.insert(keys.substr(i, 3));
        }

        

        // 利用索引_indexTable找到其在字典中对应的下标
        // 将该下标的词读入unordered_map<string, int>里
        for (const string& str : character) {
            // std::cerr << "str = " << str << '\n';
            if (_indexTable.find(str) != _indexTable.end()) {
                for(const int idx : _indexTable[str]) {
                    candidateKeys[_dict[idx].first] = _dict[idx].second;
                }
            }
        }

        
        // for (const auto& pair : candidateKeys) {
        //     std::cerr << "string = " << pair.first << '\n';
        //     std::cerr << "frq = " << pair.second << '\n';
        // }

        if (candidateKeys.size() == 0) return returnNoAnswer();

        // 对unordered_map中所有的词都做一次编辑距离操作，并放入优先队列
        priority_queue<std::tuple<string, int, int>, vector<std::tuple<string, int, int>>, decltype(&compare)> pq(compare);
        for (const auto& pair : candidateKeys) {
            int distance = utils::getInstance()->editDistance(keys, pair.first);
            pq.push(std::make_tuple(pair.first, pair.second, distance));
        }
        // 取出前10个词组成vector
        vector<string> result;
        int i = 0;
        while (!pq.empty() && i != 10) {
            result.push_back(std::get<0>(pq.top()));
            pq.pop();
            ++i;
        }

        // json返回
        if (result.size() == 0) return returnNoAnswer();

        return createJson(result);
        
    } else {
        // 英文
        vector<string> result = _enTrie->getTop10Words(keys);
        if (result.size() == 0) {
            return returnNoAnswer();
        }
        return createJson(result);
    }

   
}

string KeyRecommander::returnNoAnswer() {
    nlohmann::json root;
    nlohmann::json arr;
    root["words"] = arr;
    return root.dump();
}

string KeyRecommander::createJson(const vector<string>& result) {
    nlohmann::json root;
    nlohmann::json arr;
    for (const auto& word : result) {
        arr.push_back(word);
    }
    root["words"] = arr;
    return root.dump();    
}


KeyRecommander::KeyRecommander() {
    loadEnTrie();
    loadCnDict();
}


void KeyRecommander::loadEnTrie() {
    _enTrie = new Trie();
    const string& endict = Configuration::getInstance()->getConfigValStr("endictpath");
    ifstream ifs(endict);
    if (!ifs.is_open()) {
        LogError("endict 打不开!");
        exit(1);
    }

    string line;
    while (std::getline(ifs, line)) {
        istringstream iss(line);
        string frqstr, word;
        if (iss >> word >> frqstr) {
            _enTrie->insert(word, stoi(frqstr));
        }
    } 
    ifs.close();

}

void KeyRecommander::loadCnDict() {
    const string& cndict = Configuration::getInstance()->getConfigValStr("cndictpath");
    const string& cndictIndex = Configuration::getInstance()->getConfigValStr("cndictIndexPath");

    ifstream dictIfs(cndict);
    if (!dictIfs.is_open()) {
        LogError("cndict 打不开！\n");
        exit(1);
    }

    ifstream idxIfs(cndictIndex);
    if(!idxIfs.is_open()) {
        LogError("cndictInvert 打不开!\n");
        exit(1);
    }

    // 读取dict
    string line;
    while (std::getline(dictIfs, line)) {
        istringstream iss(line);
        string word, frqStr;
        if (iss >> word >> frqStr) {
            _dict.emplace_back(word, stoi(frqStr));
        }
    }
    dictIfs.close();

    // 读取index
    while (std::getline(idxIfs, line)) {
        istringstream iss(line);
        string word, idxStr;
        iss >> word;
        while (iss >> idxStr) {
            _indexTable[word].push_back(stoi(idxStr));
        }
    }
    idxIfs.close();

}

void KeyRecommander::destroy() {
    if (_pInstance) {
        delete _pInstance;
        _pInstance = nullptr;
    }
}



