
#include "../include/PagePreprocessor.h"
#include "../include/SplitTool.h"
#include "../include/DictProducer.h"

#include <bits/stdc++.h>
using namespace std;

int main() {
    // 模块二
    auto start = std::chrono::high_resolution_clock::now();

    PagePreprocessor processor(SplitToolCppjieba::getInstance());
    processor.xmlCleanAndCutRedundantPage();
    processor.buildInvertIndexMap();

    

    // 模块一
    DictProducer dicp(SplitToolCppjieba::getInstance());

    dicp.buildEndict();
    dicp.buildCndict();
    dicp.storeCnDict();
    dicp.buildCnDictIndex();
    dicp.storeCnDictIndex();

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Module two execution time: " << duration.count() << " ms" << '\n';

    return 0;
}