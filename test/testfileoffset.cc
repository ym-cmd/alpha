#include <iostream>
#include <fstream>
#include <string>

int main() {
    std::ifstream file("testfileoffset.txt", std::ios::in | std::ios::binary | std::ios::ate);

    if (!file.is_open()) {
        std::cerr << "Error opening file." << std::endl;
        return 1;
    }

    // 获取文件大小
    std::streampos fileSize = file.tellg();

    // 回到文件开头
    file.seekg(0, std::ios::beg);

    // 创建一个缓冲区存储文件内容
    std::string fileContent(fileSize, '\0');

    // 读取文件内容
    file.read(&fileContent[0], fileSize);

    // 关闭文件
    file.close();

    // 输出文件内容长度
    std::cout << "File length: " << fileContent.size() << " bytes" << std::endl;

    return 0;
}
