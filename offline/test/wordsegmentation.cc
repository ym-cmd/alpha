
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "../include/Configuration.h"
using namespace std;

int main() {
    const string offset = "/home/yim/cpp/boost/alpha/data/offset.dat"; 
    std::ifstream ifs(offset, std::ios::binary); // 替换为您的文件名
    if (!ifs.is_open()) {
        std::cerr << "Failed to open file." << std::endl;
        return 1;
    }

    // 定位到文件末尾
    ifs.seekg(-1, std::ios::end); // 定位到文件末尾前一个字符

    // 读取一行的最大长度
    const size_t maxLineLength = 1000; // 假设最大行长度不超过1000个字符

    // 计算需要回退多少字节才能到达上一行的开始
    int bytesToSeekBack = 1; // 已经回退了一个字节
    char ch;
    while (ifs.get(ch)) {
        if (ch == '\n') {
            break;
        }
        bytesToSeekBack++;
    }

    // 回退到上一行的开始
    ifs.seekg(-bytesToSeekBack, std::ios::cur);

    // 读取最后一行
    std::string lastLine;
    std::getline(ifs, lastLine);
    std::cout << lastLine << '\n';

    // 如果最后一行为空，则尝试读取倒数第二行
    if (lastLine.empty()) {
        ifs.seekg(-(bytesToSeekBack * 2), std::ios::cur); // 回退两行
        std::getline(ifs, lastLine);
    }

    // 去除空白字符
    lastLine.erase(0, lastLine.find_first_not_of(" \t\n\r\f\v")); // 去除行头空白
    lastLine.erase(lastLine.find_last_not_of(" \t\n\r\f\v") + 1); // 去除行尾空白

    // 解析最后一行的ID
    int id = 0;
    std::istringstream iss(lastLine);
    iss >> id;

    std::cout << "Last line ID: " << id << std::endl;

    ifs.close();
    return 0;
}