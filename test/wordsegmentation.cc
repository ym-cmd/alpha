#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;

// 
const string path = "/home/yim/cpp/boost/alpha/data/corpus/english.txt";
bool isPunctuation(char c) {
    return ispunct(c) || c == '\r'; // 判断是否为标点符号或 \r
}
int main() {
    ifstream ifs(path);
    if (!ifs.is_open()) {
        std::cerr << "Failed to open file!" << std::endl;
        return 1;
    }

    map<string, int> word_count;
    string word, line;

    while (getline(ifs, line)) {
        for (char& c : line) {
            if (isupper(c)) {
                c = tolower(c);
            }
            if (isPunctuation(c)) {
                c = ' ';
            }
        }

        stringstream ss(line);
        while (ss >> word) {
            ++word_count[word];
        }
    }

    for (const auto& entry : word_count) {
        std::cout << entry.first << ": " << entry.second << std::endl;
    }
    return 0;
}