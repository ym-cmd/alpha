#include "../include/utils.h"
#include "../include/Mylogger.h"
#include <fstream>
#include <complex>
#include <regex>

using std::ifstream;
using std::string;
using std::vector;


utils* utils::_pInstance = nullptr;
pthread_once_t utils::_once = PTHREAD_ONCE_INIT;


utils* utils::getInstance() {
    pthread_once(&_once, &init_r);
    return _pInstance;
}

utils::utils() {
    loadCnStopWords();
    loadEnStopWords();

    std::cout << "utils 初始化成功！\n";
}

void utils::loadCnStopWords() {
    const string& stopWordZhFile = Configuration::getInstance()->getConfigValStr("stop_words_zh");
    ifstream cnStopWordsIfs(stopWordZhFile);
    if (!cnStopWordsIfs.is_open()) {
        LogError("Failed to open cnStopWordEngFile\n");
        return;
    }

    string line;
    while (std::getline(cnStopWordsIfs, line)) {
        _cnStopWordList.insert(line); // 一行一个单词
    }

}
void utils::loadEnStopWords() {
    const string& stopWordEngFile = Configuration::getInstance()->getConfigValStr("stop_words_eng");
    ifstream enStopWordsIfs(stopWordEngFile);
    if (!enStopWordsIfs.is_open()) {
        LogError("Failed to open enStopWordEngFile\n");
        return;
    } 

    string line;
    while (std::getline(enStopWordsIfs, line)) {
        _enStopWordList.insert(line); // 一行一个单词
    }

}


void utils::destroy() {
    if (_pInstance) {
        delete _pInstance;
        _pInstance = nullptr;
    }
    // std::cout << "utils destroy\n";
}

// init_r在pthread_once, 只会被执行一次
void utils::init_r() {
    _pInstance = new utils();
    atexit(destroy); // 注册destroy()，结束后自动调用进行析构
}


unordered_map<string, int> utils::filterStopWords(const std::vector<string>& vec) {
    unordered_map<string, int> result;
    for (const auto& str : vec) {
        if (_cnStopWordList.find(str) == _cnStopWordList.end()) {
            ++result[str];
        }
    }
    return result;
}

double utils::TF_IDF(int TF, int DF, size_t N) {
    return std::log(static_cast<double>(N) / (1 + DF)) * TF;
}
              
string utils::extractTagContent(const string& doc, const string& tag) {

    std::regex re("<" + tag + ">(.*?)</" + tag + ">");
    std::smatch match;
    if (std::regex_search(doc, match, re)) {
        return match[1];
    }
    return "";

    // string start_tag = "<content>";
    // string end_tag = "</content>";

    // size_t start_pos = content.find(start_tag);
    // if (start_pos == std::string::npos) {
    //     return "";  // 找不到标签
    // }

    // start_pos += start_tag.length();  // 跳过起始标签
    // size_t end_pos = content.find(end_tag, start_pos);
    // if (end_pos == std::string::npos) {
    //     return "";  // 找不到结束标签
    // }

    // // 提取标签中的内容
    // return content.substr(start_pos, end_pos - start_pos);
}

std::size_t utils::length(const std::string& str) {
    size_t ilen = 0;
    for (size_t idx = 0; idx != str.size(); ++idx) {
        int nBytes = nBytesCode(str[idx]);
        idx += (nBytes - 1);
        ++ilen;
    }
    return ilen;
}

size_t utils::nBytesCode(const char ch) {
    if (ch & (1 << 7)) {
        int nBytes = 1;
        for (int idx = 0; idx != 6; ++idx) {
            if (ch & (1 << (6 - idx))) {
                ++nBytes;
            } else {
                break;
            }
        }
        return nBytes;
    }
    return 1;
}

int utils::editDistance(const string& lhs, const string& rhs) {
    size_t lhs_len = length(lhs);
    size_t rhs_len = length(rhs);
    vector<std::vector<int>> editDist(lhs_len + 1, std::vector<int>(rhs_len + 1));

    for (size_t idx = 0; idx <= lhs_len; ++idx) {
        editDist[idx][0] = idx;
    }

    for (size_t idx = 0; idx <= rhs_len; ++idx) {
        editDist[0][idx] = idx;
    }

    std::string sublhs, subrhs;
    for (size_t dist_i = 1, lhs_idx = 0; dist_i <= lhs_len; ++dist_i, ++lhs_idx) {
        size_t nBytes = nBytesCode(lhs[lhs_idx]);
        sublhs = lhs.substr(lhs_idx, nBytes);
        lhs_idx += (nBytes - 1);

        for (size_t dist_j = 1, rhs_idx = 0; dist_j <= rhs_len; ++dist_j, ++rhs_idx) {
            nBytes = nBytesCode(rhs[rhs_idx]);
            subrhs = rhs.substr(rhs_idx, nBytes);
            rhs_idx += (nBytes - 1);

            if (sublhs == subrhs) {
                editDist[dist_i][dist_j] = editDist[dist_i - 1][dist_j - 1];
            } else {
                editDist[dist_i][dist_j] = std::min({ 
                    editDist[dist_i - 1][dist_j] + 1, 
                    editDist[dist_i][dist_j - 1] + 1,
                    editDist[dist_i - 1][dist_j - 1] + 1 
                });
            }
        }
    }
    return editDist[lhs_len][rhs_len];
}

bool utils::isChinese(const std::string& word) {
    // 判断字符串是否为中文字符
    for (char c : word) {
        if (c & 0x80) { // 如果高位为1，可能是中文字符
            return true;
        }
    }
    return false;
}

void utils::addHttpHead(string& str) {
    const string startLine = "HTTP/1.1 200 OK\r\n";
    const string headers = "Server: MyHttpServer1.0\r\n"
                        "Content-Type: text/html\r\n"
                        "Content-Length: ";
    const string emptyLine = "\r\n";
    str = startLine + headers + std::to_string(str.size()) + "\r\n" + emptyLine + str;
}