#include <asm-generic/socket.h>
#include <func.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <bits/stdc++.h>
#include "../include/nlohmann/json.hpp"

using namespace std;

class KeyRecommander {
public:
    // 返回空结果的JSON
    std::string returnNoAnswer() {
        nlohmann::json root;
        nlohmann::json arr;
        root["words"] = arr;
        return root.dump();  // 转换为JSON字符串返回
    }

    // 生成包含推荐词的JSON
    std::string createJson(const std::vector<std::string>& result) {
        nlohmann::json root;
        nlohmann::json arr;
        for (const auto& word : result) {
            arr.push_back(word);  // 添加推荐词
        }
        root["words"] = arr;
        return root.dump();
    }
};

class WebPageQuery {
public:
    // 根据文档ID和文档内容生成JSON
    std::string createJson(const std::vector<int>& topDocIDs, const std::vector<std::string>& docs) {
        nlohmann::json root;
        nlohmann::json arr = nlohmann::json::array();
        for (size_t i = 0; i < topDocIDs.size(); ++i) {
            if (i >= docs.size()) continue;

            // 解析字符串内容
            std::string title = extractTagContent(docs[i], "title");
            std::string link = extractTagContent(docs[i], "link");
            std::string content = extractTagContent(docs[i], "content");

            nlohmann::json elem;
            elem["title"] = title;
            elem["url"] = link;
            elem["content"] = content;

            arr.push_back(elem);
        }
        root["files"] = arr;
        return root.dump();
    }

    // 返回空结果的JSON
    std::string returnNoAnswer() {
        nlohmann::json root;
        nlohmann::json arr;
        nlohmann::json elem;

        elem["title"] = "404, not found";
        elem["summary"] = "未找到你搜索的内容";
        elem["url"] = "";

        arr.push_back(elem);

        root["files"] = arr;
        return root.dump();
    }

private:
    // 假设有一个解析标签内容的函数
    std::string extractTagContent(const std::string& doc, const std::string& tag) {
        // 简单模拟提取标签内容的逻辑
        size_t start = doc.find("<" + tag + ">");
        size_t end = doc.find("</" + tag + ">");
        if (start != std::string::npos && end != std::string::npos) {
            return doc.substr(start + tag.length() + 2, end - start - tag.length() - 2);
        }
        return "";
    }
};


int readn(char* buf, int len, int _fd) {
    int left = len;
    char* pstr = buf;
    int ret = 0;

    while (left > 0) {
        ret = read(_fd, pstr, left);
        if (ret == -1 && errno == EINTR) {
            continue;
        } else if (ret == -1) {
            perror("read error -1");
            return -1;
        } else if (ret == 0) {
            break; // 退出
        } else {
            pstr += ret;
            left -= ret;
        }
    }

    return left - ret;
}

int readLine(char* buf, int len, int _fd) {
    int left = len - 1;
    char* pstr = buf;
    int ret = 0, total = 0;

    while (left > 0) {
        ret = recv(_fd, pstr, left, MSG_PEEK);
        if (ret == -1 && errno == EINTR) {
            continue;
        } else if (ret == -1) {
            perror("readLine error -1");
            return -1;
        } else if (ret == 0) {
            break;
        } else {
            for (int idx = 0; idx < ret; ++idx) {
                if (pstr[idx] == '\n') {
                    int sz = idx + 1;
                    readn(pstr, sz, _fd);
                    pstr += sz;
                    *pstr = '\0';
                    return total + sz;
                }
            }

            readn(pstr, ret, _fd);
            total += ret;
            pstr += ret;
            left -= ret;
        }
    }

    *pstr = '\0';
    return total - left;
}


void ERROR_CHECK(int a, int b, string c) {
    if (a == b) {
        cout << c << '\n';
        exit(1);
    }
}
int main()
{
    // 创建监听套接字
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    ERROR_CHECK(listenfd, -1, "socket");

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));//初始化
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(8888);
    serveraddr.sin_addr.s_addr = inet_addr("192.168.137.146");

    // 设置网络地址可以重用
    int on = 1;
    int ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    ERROR_CHECK(ret, -1, "setsockopt");

    // 绑定网络地址
    ret = bind(listenfd, 
                   (const struct sockaddr*)&serveraddr, 
                   sizeof(serveraddr));
    ERROR_CHECK(ret, -1, "bind");

    // 进行监听
    ret = listen(listenfd, 10);
    ERROR_CHECK(ret, -1, "listen");

    while(1)
    {
        struct sockaddr_in clientAddr;
        memset(&clientAddr, 0, sizeof(clientAddr));
        socklen_t len = sizeof(clientAddr);
        int peerfd = accept(listenfd, (struct sockaddr*)&clientAddr, &len);
        printf("%s:%d has connected.\n", 
               inet_ntoa(clientAddr.sin_addr),
               ntohs(clientAddr.sin_port));

        // 1. 获取HTTP请求报文
        char buff[4096] = {0};
        int ret = recv(peerfd, buff, sizeof(buff), 0);
        printf("ret: %d bytes.\n%s\n", ret, buff);

        // 2. 解析HTTP请求并读取对应文件
        const char * startLine = "HTTP/1.1 200 OK\r\n";
        const char * headers = "Server: MyHttpServer1.0\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: ";
        const char * emptyLine = "\r\n";

        ifstream ifs("index.html", ios::binary | ios::ate);
        streamsize fileSize = ifs.tellg();
        ifs.seekg(0, ios::beg);

        char* buffer = new char[fileSize + 1];
        ifs.read(buffer, fileSize);
        ifs.close();
        buffer[fileSize] = '\0';

        memset(buff, 0, sizeof(buff));
        sprintf(buff, "%s%s%ld\r\n%s%s", startLine, headers, strlen(buffer), emptyLine, buffer);
        printf("response:\r\n%s\n", buff);
        ret = send(peerfd, buff, strlen(buff), 0);
        printf("\nsend %d bytes.\n", ret);
        
        while (1) {
            char buffer[4096] = {0};
            readLine(buffer, 4096, peerfd);
            printf("%s\n", buffer);
        }

        

        close(peerfd);
    }

    close(listenfd);

    return 0;
}
