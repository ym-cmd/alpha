#include "trie.h"

#include <vector>
#include <set>
#include <unordered_map>

// 饿汉模式单例
class KeyRecommander {
public:
    static KeyRecommander* getInstance();
    std::string doQuery(const std::string& keys);

private:
    void loadEnTrie();
    void loadCnDict();
    std::string returnNoAnswer();
    std::string createJson(const std::vector<std::string>& result);
    static bool compare(const std::tuple<std::string, int, int>& a, const std::tuple<std::string, int, int>& b);

private:
    KeyRecommander();
    static void destroy();

    KeyRecommander(const KeyRecommander& rhs) = delete;
    KeyRecommander& operator=(const KeyRecommander& rhs) = delete;

private:
    static KeyRecommander* _pInstance;

    Trie* _enTrie;

    // 词 词频
    std::vector<std::pair<std::string, int>> _dict;
    // 字 字出现的下标
    std::unordered_map<std::string, std::vector<int>> _indexTable;
};