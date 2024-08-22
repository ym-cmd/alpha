#include <unordered_map>
#include <string>
#include <vector>
#include <queue>


class TrieNode {
public:
    std::unordered_map<char, TrieNode*> _children;
    std::string _word;
    int _frequency;

    TrieNode() : _frequency(0) {}
};

class Trie {
public:
    Trie();
    ~Trie();

    void insert(const std::string& word, int frequency);

    std::vector<std::string> getTop10Words(const std::string& prefix);

private:
    static bool compare(const std::pair<int, std::string>& a, const std::pair<int, std::string>& b) {
        return a.first > b.first;
    }

    // 深度优先搜索，找到所有以当前节点为前缀的单词
    void dfs(TrieNode* node, std::priority_queue<std::pair<int, std::string>, std::vector<std::pair<int, std::string>>, decltype(&compare)>& pq);


private:
    TrieNode* _root;
};