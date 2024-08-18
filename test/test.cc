#include <iostream>
#include <fstream>
#include <vector>
//this define can avoid some logs which you don't need to care about.
#define LOGGER_LEVEL LL_WARN 

#include "../include/simhash/simhash/Simhasher.hpp"

using namespace simhash;
using std::string;
using std::cout;
using std::vector;
using std::pair;
using std::endl;

int main(int argc, char** argv)
{
    // 构建一个simhasher
    Simhasher simhasher("../include/simhash/dict/jieba.dict.utf8", "../include/simhash/dict/hmm_model.utf8", "../include/simhash/dict/idf.utf8", "../include/simhash/dict/stop_words.utf8");
    // s是文章
    string s("我是蓝翔技工拖拉机学院手扶拖拉机专业的。不用多久，我就会升职加薪，当上总经理，出任CEO，走上人生巅峰。");
    size_t topN = 5;
    uint64_t u64 = 0; // 将来去存储指纹的
    simhasher.make(s, topN, u64); // 算指纹
    cout<< "文本：\"" << s << "\"" << endl;
    cout<< "simhash值是: " << u64<<endl;

    const char * bin1 = "100010110110";
    const char * bin2 = "110001110011";

    uint64_t u1, u2;
    u1 = Simhasher::binaryStringToUint64(bin1); 
    u2 = Simhasher::binaryStringToUint64(bin2); 
    
    cout<< bin1 << "和" << bin2 << " simhash值的相等判断如下："<<endl;
    // Simhasher::isEqual 对比指纹
    cout<< "海明距离阈值默认设置为3，则isEqual结果为：" << (Simhasher::isEqual(u1, u2)) << endl; 
    cout<< "海明距离阈值默认设置为5，则isEqual结果为：" << (Simhasher::isEqual(u1, u2, 5)) << endl; 
    return EXIT_SUCCESS;
}