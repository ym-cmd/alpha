#include "../include/net/SocketIO.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <cstring>
#include <string>
#include <iostream>

using namespace std;

SocketIO::SocketIO(int fd)
: _fd(fd)
{

}

SocketIO::~SocketIO()
{
    /* close(_fd); */
}

//len = 10000    1500/6     1000/1
int SocketIO::readn(char *buf, int len)
{
    int left = len;
    char *pstr = buf;
    int ret = 0;

    while(left > 0)
    {
        ret = read(_fd, pstr, left);
        if(-1 == ret && errno == EINTR)
        {
            continue;
        }
        else if(-1 == ret)
        {
            perror("read error -1");
            return -1;
        }
        else if(0 == ret)
        {
            break;
        }
        else
        {
            pstr += ret;
            left -= ret;
        }
    }

    return len - left;
}

int SocketIO::readLine(char *buf, int len)
{
    int left = len - 1;
    char *pstr = buf;
    int ret = 0, total = 0;

    while(left > 0)
    {
        //MSG_PEEK不会将缓冲区中的数据进行清空,只会进行拷贝操作
        ret = recv(_fd, pstr, left, MSG_PEEK);
        if(-1 == ret && errno == EINTR)
        {
            continue;
        }
        else if(-1 == ret)
        {
            perror("readLine error -1");
            return -1;
        }
        else if(0 == ret)
        {
            break;
        }
        else
        {
            for(int idx = 0; idx < ret; ++idx)
            {
                if(pstr[idx] == '\n')
                {
                    int sz = idx + 1;
                    readn(pstr, sz);
                    pstr += sz;
                    *pstr = '\0';//C风格字符串以'\0'结尾

                    return total + sz;
                }
            }

            readn(pstr, ret);//从内核态拷贝到用户态
            total += ret;
            pstr += ret;
            left -= ret;
        }
    }
    *pstr = '\0';

    return total - left;

}

int SocketIO::writen(const char *buf, int len)
{
    int left = len;
    const char *pstr = buf;
    int ret = 0;

    while(left > 0)
    {
        ret = write(_fd, pstr, left);
        if(-1 == ret && errno == EINTR)
        {
            continue;
        }
        else if(-1 == ret)
        {
            perror("writen error -1");
            return -1;
        }
        else if(0 == ret)
        {
            break;
        }
        else
        {
            pstr += ret;
            left -= ret;
        }
    }
    return len - left;
}

int SocketIO::readHttp(char* buf, int len) {
    std::cerr << "readHttp begin\n";
    int total_read = 0;
    int bytes_read = 0;

    // 读取请求头
    while (total_read < len) {
        bytes_read = read(_fd, buf + total_read, len - total_read);
        if (bytes_read <= 0) {
            return bytes_read;  // 出错或连接关闭
        }
        total_read += bytes_read;

        // 检查请求头结束的标志 "\r\n\r\n"
        if (strstr(buf, "\r\n\r\n") != nullptr) {
            break;  // 读取到请求头的结束部分
        }
    }

    // 解析 Content-Length 字段
    int content_length = 0;
    char* content_length_str = strstr(buf, "Content-Length: ");
    if (content_length_str != nullptr) {
        content_length_str += strlen("Content-Length: ");
        content_length = stoi((string)content_length_str);
    }

    // 计算请求体应有的总长度
    int header_length = strstr(buf, "\r\n\r\n") - buf + 4;
    int total_length = header_length + content_length;

    // 读取请求体
    while (total_read < total_length && total_read < len) {
        bytes_read = read(_fd, buf + total_read, len - total_read);
        if (bytes_read <= 0) {
            return bytes_read;  // 出错或连接关闭
        }
        total_read += bytes_read;
    }

    std::cerr << "readHttp end\n";
    return total_read;
    
}

int SocketIO::readHttpBody(char* buf, int len) {
    int contentLength = 0;
    char line[1024] = {0};

    // 逐行读取 HTTP 请求头
    while (true) {
        int lineLength = readLine(line, sizeof(line));
        if (lineLength <= 0) {
            return -1; // 读取失败
        }    

        // 查找 Content-Length
        if (strncmp(line, "Content-Length:", 15) == 0) {
            contentLength = atoi(line + 15);
        }

        // 如果遇到空行，说明请求头部分结束
        if (strcmp(line, "\r\n") == 0) {
            break;
        }
    }

    // 如果 Content-Length 没有找到，返回错误
    if (contentLength == 0 || contentLength >= len) {
        return -1;
    }

    // 读取 Body 内容
    int ret = readn(buf, contentLength);
    if (ret != contentLength) {
        return -1;
    }

    buf[ret] = '\0'; // 确保缓冲区以空字符结束
    return ret;
}