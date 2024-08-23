#include <bits/stdc++.h>
using namespace std;
void ERROR_CHECK(int a, int b, string c) {
    if (a == b) {
        cout << c << '\n';
        exit(1);
    }
}
int main()
{
    //创建监听套接字
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    ERROR_CHECK(listenfd, -1, "socket");

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));//初始化
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(8080);
    serveraddr.sin_addr.s_addr = inet_addr("192.168.137.146");

    //设置网络地址可以重用
    int on = 1;
    int ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    ERROR_CHECK(ret, -1, "setsockopt");

    //绑定网络地址
    ret = bind(listenfd, 
                   (const struct sockaddr*)&serveraddr, 
                   sizeof(serveraddr));
    ERROR_CHECK(ret, -1, "bind");

    //进行监听
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

        //1.获取http请求报文
        char buff[4096] = {0};
        int ret = recv(peerfd, buff, sizeof(buff), 0);
        printf("ret: %d bytes.\n%s\n", ret, buff);

        //2.对请求报文进行响应操作
        const char * startLine = "HTTP/1.1 200 OK\r\n";
        const char * headers = "Server: MyHttpServer1.0\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: ";
        const char * emptyLine = "\r\n";
        ifstream ifs("index.html", std::ios::binary | std::ios::ate);

        std::streamsize fileSize = ifs.tellg(); // 获取文件指针当前位置，即文件大小
        ifs.seekg(0, std::ios::beg); // 重置文件指针到文件开始位置

        // 分配内存
        char* buffer = new char[fileSize + 1]; // +1 是为了添加 null 终止符

        // 读取文件内容
        ifs.read(buffer, fileSize);
        ifs.close();

        // 添加 null 终止符
        buffer[fileSize] = '\0';

        const char * body = buffer;
        //清空缓冲区
        memset(buff, 0, sizeof(buff));
        sprintf(buff, "%s%s%ld\r\n%s%s",
                startLine,headers,strlen(body), emptyLine,body);
        printf("response:\r\n%s\n", buff);
        ret = send(peerfd, buff, strlen(buff), 0);
        printf("\nsend %d bytes.\n", ret);


        // 这里有一个死循环，一直收响应, 直到网络连接断开
        while (1) {
            // 收http响应

            // 解析json， 并将json中的内容输出出来

            // 处理json，（这里不做处理，推荐就直接返回json里的内容，搜索就返回returnNoAnswer()

        }
       
    }

    close(listenfd);


    return 0;
}

