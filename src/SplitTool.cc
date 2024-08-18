#include "../include/SplitTool.h"
#include "../include/Configuration.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <unordered_map>

using std::ifstream; 
using std::ofstream;
using std::stringstream; 

namespace fs = std::filesystem;

const char* const DICT_PATH = "../include/simhash/dict/jieba.dict.utf8";
const char* const HMM_PATH = "../include/simhash/dict/hmm_model.utf8";
const char* const USER_DICT_PATH = "../include/simhash/dict/user.dict.utf8";
const char* const IDF_PATH = "../include/simhash/dict/idf.utf8";
const char* const STOP_WORD_PATH = "../include/simhash/dict/stop_words.utf8";


SplitToolCppjieba* SplitToolCppjieba::_pInstance = SplitToolCppjieba::getInstance();

SplitToolCppjieba* SplitToolCppjieba::getInstance() {
    if (_pInstance == nullptr) {
        atexit(destroy);
        _pInstance = new SplitToolCppjieba();
    }
    return _pInstance;
}


// 对每个文件进行清洗换行符，英文词
vector<string> SplitToolCppjieba::cut(const string& cnfileContent){

    // 分词后的中文语料
    vector<string> result;

    // 使用cppjieba进行分词
    _jieba->Cut(cnfileContent, result, true);

    return result;
}

SplitToolCppjieba::SplitToolCppjieba()
: _jieba(new cppjieba::Jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH))
{}

void SplitToolCppjieba::destroy() {
    if (_pInstance) {
        delete _pInstance;
        _pInstance = nullptr;
    }
}

SplitToolCppjieba::~SplitToolCppjieba() {
    if (_jieba) {
        delete _jieba;
        _jieba = nullptr;
    }
}