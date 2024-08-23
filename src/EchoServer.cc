#include "../include/net/EchoServer.h"
#include "../include/net/EventLoop.h"
#include "../include/net/TcpConnection.h"
#include "../include/utils.h"
#include <iostream>
#include <functional>
#include <sstream>
#include <fstream>

using namespace std;
using std::cout;
using std::endl;
using std::bind;

MyTask::MyTask(const string &msg, const TcpConnectionPtr &con)
: _msg(msg)
, _con(con)
{

}

std::string parseHttpBody(const std::string& httpMsg) {
    int contentLength = 0;
    size_t headersEndPos = httpMsg.find("\r\n\r\n"); // 查找头部结束位置
    if (headersEndPos == std::string::npos) {
        return ""; // 未找到头部结束位置
    }

    // 逐行解析头部
    size_t startPos = 0;
    size_t endPos = 0;
    while (endPos != std::string::npos) {
        endPos = httpMsg.find("\r\n", startPos);
        if (endPos != std::string::npos) {
            std::string header = httpMsg.substr(startPos, endPos - startPos);
            if (header.find("Content-Length:") == 0) {
                contentLength = atoi(header.c_str() + 15);
            }
            startPos = endPos + 2; // 跳过 "\r\n"
        }
    }

    // 如果 Content-Length 没有找到，返回空字符串
    if (contentLength == 0) {
        return "";
    }

    // 读取 Body 内容
    std::string body = httpMsg.substr(headersEndPos + 4, contentLength); // 跳过 "\r\n\r\n"

    return body;
}

void MyTask::process()
{   
    std::cerr << "process!\n";
    //_con->sendInLoop(_msg);
    //处理业务逻辑
    
    // _msg 为整个http请求
    // 解析_msg

    istringstream iss(_msg);
    string method;
    iss >> method;
    // 如果是GET请求,则返回整个index.html, // 如果是POST请求，则返回body内容
    if (method == "GET") {
        std::ifstream ifs("index.html");
        if (!ifs.is_open()) {
            std::cerr << "ifs worng!\n";
            exit(1);
        }
        std::stringstream ss;
        ss << ifs.rdbuf();
    
        string msg = ss.str();

        // 加http头部
        utils::getInstance()->addHttpHead(msg);

        _con->sendInLoop(msg);
    } else if (method == "POST") {
       _msg = parseHttpBody(_msg);
       _con->parserInLoop(_msg);
    } 

    std::cerr << "_msg : " <<_msg << "\n----msg-----\n";
}

EchoServer::EchoServer(size_t threadNum, size_t queSize
                       , const string &ip
                       , unsigned short port)
: _pool(threadNum, queSize)
, _server(ip, port)
{

}

EchoServer::~EchoServer()
{

}

//服务器的启动与停止
void EchoServer::start()
{
    _pool.start();

    //注册所有的回调
    using namespace std::placeholders;
    _server.setAllCallback(std::bind(&EchoServer::onNewConnection, this, _1)
                           , std::bind(&EchoServer::onMessage, this, _1)
                           , std::bind(&EchoServer::onClose, this, _1));

    _server.start();
}

void EchoServer::stop()
{
    _pool.stop();
    _server.stop();

}

void EchoServer::onNewConnection(const TcpConnectionPtr &con)
{
    cerr << con->toString() << " has connected!!!" << endl;

}
void EchoServer::onMessage(const TcpConnectionPtr &con)
{
    //接收客户端的数据
    //
    string msg = con->receive();
    cout << ">>recv msg from client: " << msg << endl;

    MyTask task(msg, con);

    _pool.addTask(std::bind(&MyTask::process, task));
}

void EchoServer::onClose(const TcpConnectionPtr &con)
{
    cout << con->toString() << " has closed!!!" << endl;
}
