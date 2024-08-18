
#include "../include/Configuration.h"
#include "../include/DictProducer.h"
#include "../include/SplitTool.h"
#include <bits/stdc++.h>
using namespace std;

int main() {
    // 
    DictProducer dicp(SplitToolCppjieba::getInstance());

    dicp.buildEndict();
    dicp.buildCndict();
    dicp.storeCnDict();
    dicp.buildCnDictIndex();
    dicp.storeCnDictIndex();

    return 0;
}