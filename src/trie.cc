#include "../include/trie.h"
#include <algorithm>
#include <functional>

using std::priority_queue;
using std::pair;
using std::string;
using std::vector;


Trie::Trie() {
    _root = new TrieNode();
}

Trie::~Trie() {
    // 递归删除所有节点
    std::function<void(TrieNode*)> deleteNode = [&](TrieNode* node) {
        for (auto& child : node->_children) {
            deleteNode(child.second);
        }
        delete node;
    };
    deleteNode(_root);
}


void Trie::insert(const std::string& word, int frequency) {
    TrieNode* node = _root;
    for (char c : word) {
        if (node->_children.find(c) == node->_children.end()) {
            node->_children[c] = new TrieNode();
        }
        node = node->_children[c];
    }

    node->_frequency = frequency;
    node->_word = word;
}

// 深度优先找到所有以当前节点为前缀的单词
void Trie::dfs(TrieNode* node, priority_queue<pair<int, string>, vector<pair<int, string>>, decltype(&compare)>& pq) {
    if (node->_frequency > 0) {
        pq.emplace(node->_frequency, node->_word);
        if (pq.size() > 10) {
            pq.pop();
        }
    }

    for (auto& elem : node->_children) {
        dfs(elem.second, pq);
    }

}

vector<string> Trie::getTop10Words(const string& prefix) {
    TrieNode* node = _root;
    for (char c : prefix) {
        // 没找到以当前为前缀的
        if (node->_children.find(c) == node->_children.end()) {
            return {};
        }
        node = node->_children[c]; // 一直遍历到末尾, 因为要找以这个词做前缀的词
    }

    priority_queue<pair<int, string>, vector<pair<int, string>>, decltype(&compare)> pq(compare);
    dfs(node, pq);

    vector<string> result;
    while (!pq.empty()) {
        result.push_back(pq.top().second);
        pq.pop();
    }
    std::reverse(result.begin(), result.end());
    return result;
}