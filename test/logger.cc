// #include "../include/Mylogger.h"
#include <bits/stdc++.h>
using namespace std;

string extractTagContent(const string& doc, const string& tag) {
    regex re("<" + tag + ">(.*?)</" + tag + ">");
    smatch match;
    if (regex_search(doc, match, re)) {
        return match[1];
    }
    return "";
}

int main() {
    string s = "我是中国人";
    vector<string> vec;
    for (size_t i = 0; i < s.size(); i += 3) {
        vec.push_back(s.substr(i, 3));
    }

    for (auto x : vec) {
        cout << x << '\n';
    }

    return 0;
}