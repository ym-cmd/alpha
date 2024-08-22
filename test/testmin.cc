#include <bits/stdc++.h>
using namespace std;
size_t nBytesCode(const char ch) {
    if (ch & 0x80) {
        if ((ch & 0xE0) == 0xC0) return 2; // 2字节
        if ((ch & 0xF0) == 0xE0) return 3; // 3字节
        if ((ch & 0xF8) == 0xF0) return 4; // 4字节
    }
    return 1; // 1字节
}

std::size_t length(const std::string& str) {
    size_t ilen = 0;
    for (size_t idx = 0; idx != str.size(); ++idx) {
        int nBytes = nBytesCode(str[idx]);
        idx += (nBytes - 1);
        ++ilen;
    }
    return ilen;
}
int editDistance(const std::string& lhs, const std::string& rhs) {
    size_t lhs_len = length(lhs);
    size_t rhs_len = length(rhs);
    std::vector<std::vector<int>> editDist(lhs_len + 1, std::vector<int>(rhs_len + 1));

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



int main() {
    std::string target = "水";
    std::vector<std::string> test_strings = {"平", "准", "浒传", "浒", "波不兴", "火不相容", "水流", "河水", "水珠"};

    for (const auto& test_str : test_strings) {
        int distance = editDistance(target, test_str);
        std::cout << "The edit distance between \"" << target << "\" and \"" << test_str << "\" is: " << distance << std::endl;
    }

    return 0;

}