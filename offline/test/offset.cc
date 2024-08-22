#include <iostream>
#include <fstream>

int main() {
    std::ifstream file("/home/yim/cpp/boost/alpha/data/ripepage.dat", std::ios::binary | std::ios::ate); // 以二进制模式打开文件并将文件指针移动到末尾
    if (file.is_open()) {
        std::streamsize fileSize = file.tellg(); // 获取文件大小
        std::cout << "File size: " << fileSize << " bytes" << std::endl;
        file.close();
    } else {
        std::cerr << "Unable to open the file!" << std::endl;
    }
    return 0;
}
