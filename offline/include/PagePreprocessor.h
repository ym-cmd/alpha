#pragma once

#include "SplitTool.h"
#include <unordered_map>

using std::unordered_map;
using std::pair;

class PagePreprocessor {
public:
    PagePreprocessor(SplitTool* cuttor);
    // 清洗xml中的数据，然后保存为ripepage.dat和offset.dat
    void xmlClean();
    void cutRedundantPage();
    void buildInvertIndexMap();
    void xmlCleanAndCutRedundantPage();

private:
    SplitTool* _cuttor;
    size_t _nodupMaxid = 0;
};