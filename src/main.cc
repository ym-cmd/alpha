
#include "../include/PagePreprocessor.h"

#include <bits/stdc++.h>
using namespace std;

int main() {
    // 模块二
    PagePreprocessor processor;
    processor.xmlClean();
    processor.cutRedundantPage();



    // 模块一
    // DictProducer dicp(SplitToolCppjieba::getInstance());

    // dicp.buildEndict();
    // dicp.buildCndict();
    // dicp.storeCnDict();
    // dicp.buildCnDictIndex();
    // dicp.storeCnDictIndex();

    return 0;
}