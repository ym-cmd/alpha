#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

// 删除换行符的函数，并加速中文和英文的处理
void removeNewlinesAndOptimize(std::string& str) {
    int len = str.size();
    int j = 0;

    for (int i = 0; i < len;) {
        if (str[i] == '\n') {
            // 如果是换行符，跳过
            ++i;
        } else if ((str[i] & 0x80) == 0) {
            // 如果是英文字符 (ASCII)，直接复制并移动到下一个字符
            str[j++] = str[i++];
        } else {
            // 如果是中文字符 (UTF-8)，假设是三字节编码，复制三个字节
            if (i + 2 < len) {
                str[j++] = str[i];
                str[j++] = str[i + 1];
                str[j++] = str[i + 2];
            }
            i += 3; // 移动到下一个字符（中文字符是3字节的）
        }
    }

    str.resize(j);  // 调整字符串大小，删除多余部分

}

void testFunction() {
    // 测试数据
    std::string testData = "Hello\n世界 你好\nWor ld\n你好";
    std::string expected = "Hello世界你好World你好";

    // 处理数据
    removeNewlinesAndOptimize(testData);

    // 验证结果


    std::cout << "Test passed: " << testData << std::endl;
}

int main() {
    testFunction();
    return 0;
}