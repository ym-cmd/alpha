// #include "../include/Mylogger.h"
#include <bits/stdc++.h>
using namespace std;
size_t getByteNum_UTF8(const char byte) {
    int byteNum = 0;
    for (size_t i = 0; i < 6; ++i) {
        if (byte & (1 << (7 - i))) 
            ++byteNum;
        else 
            break;
    }

    return byteNum == 0 ? 1 : byteNum;
}

int main() {
    string word = "我们是他们";
    size_t charNums = word.size()/getByteNum_UTF8(word[0]);

    for (size_t idx = 0, n = 0; n != charNums; ++idx, ++n) {
        size_t charlen = getByteNum_UTF8(word[idx]);
        string subword = word.substr(idx, charlen);

        std::cerr << subword << '\n';
        idx += (charlen - 1);
    }

    return 0;
}