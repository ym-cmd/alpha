#pragma once

#include <unordered_map>

using std::unordered_map;
using std::pair;

class PagePreprocessor {
public:
    // 清洗xml中的数据，然后保存为ripepage.dat和offset.dat
    void xmlClean();
    void cutRedundantPage();
    void buildInvertIndexMap();

private:
    size_t _maxid = 0;
    size_t _nodupMaxid = 0;
};