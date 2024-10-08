#include "../include/net/TcpConnection.h"
#include "../include/net/EventLoop.h"
#include "../include/WebPageQuery.h"
#include "../include/KeyRecommander.h"
#include "../include/nlohmann/json.hpp"
#include "../include/utils.h"
#include "../include/redis.h"

#include <iostream>
#include <sstream>

using std::cout;
using std::endl;
using std::ostringstream;

TcpConnection::TcpConnection(int fd, EventLoop *loop)
: _loop(loop)
, _sockIO(fd)
, _sock(fd)
, _localAddr(getLocalAddr())
, _peerAddr(getPeerAddr())
{

}

TcpConnection::~TcpConnection()
{

}

void TcpConnection::send(const string &msg)
{
    _sockIO.writen(msg.c_str(), msg.size());
}

//到底需要干什么?
//此处参数中的msg就是线程池中的线程处理好之后的
//可以发送给客户端的msg
//还需要将发送数据能力的TcpConnection的对象也发给
//Reactor，只有这样才能让Reactor具备发送数据的能力
//
//sendInLoop函数需要将线程池中的线程处理好之后的数据
//msg，以及发送数据的连接TcpConnection的对象，以及
//发送数据的能力函数send都一次性打包发给EventLoop
//那么EventLoop就可以具备发送数据的能力，也就可以
//将数据发给客户端

void TcpConnection::parserInLoop(const string& msg) {
    if (_loop) {
        _loop->runInLoop(std::bind(&TcpConnection::parser, this, msg));
    }
}
void TcpConnection::parser(const string& msg) {
    auto json_msg = nlohmann::json::parse(msg);

    if (!json_msg.contains("query") || !json_msg.contains("type")) {
        std::cerr << "Invalid JSON format: missing 'query' or 'type'" << std::endl;
        exit(1);
    }

    // 获取类型和查询内容
    std::string type = json_msg["type"];
    std::string query = json_msg["query"];
    
    std::cerr << "type = " << type << '\n';
    std::cerr << "query = " << query << '\n';
    // 写入redis缓存，下一次查到query的时候就先查redis，有就返回result，否则继续查 query : result
    redisContext* context = Redis::getInstance()->getcontext();
    string result;
    if (type == "1") { // 推荐
        // 封装query
        string redisQuery = "Recommand" + query;

        // 查看redis是否命中
        redisReply* reply = (redisReply*)redisCommand(context, "GET %s", redisQuery.c_str());

        if (reply == nullptr) {
            std::cerr << "Redis command failed: " << context->errstr << std::endl;
            exit(1);
        }

        if (reply->type == REDIS_REPLY_STRING) {
            result = reply->str;
            freeReplyObject(reply);
            std::cerr << "--------redis命中--------\n";
        } else { 
            // 没命中
            std::cerr << "--------redis没命中--------\n";
            result = KeyRecommander::getInstance()->doQuery(query);
            // 加上http请求头
            utils::getInstance()->addHttpHead(result);
            // 写入redis
            freeReplyObject(reply);
            reply = (redisReply*)redisCommand(context, "SET %s %s", redisQuery.c_str(), result.c_str());

            if (reply == nullptr) {
                std::cerr << "Redis SET command failed: " << context->errstr << std::endl;
                exit(1);
            }
        
            freeReplyObject(reply);
        }

    } else if (type == "2") { // 搜索
        // 封装query
        string redisQuery = "Search" + query;

        // 查看redis是否命中
        redisReply* reply = (redisReply*)redisCommand(context, "GET %s", redisQuery.c_str());

        if (reply == nullptr) {
            std::cerr << "Redis command failed: " << context->errstr << std::endl;
            exit(1);
        }

        if (reply->type == REDIS_REPLY_STRING) {
            result = reply->str;
            freeReplyObject(reply);
            std::cerr << "--------redis命中--------\n";
        } else { 
            // 没命中
            std::cerr << "--------redis没命中--------\n";
            result = WebPageQuery::getInstance()->doQuery(query);
            // 加上http请求头
            utils::getInstance()->addHttpHead(result);
            // 写入redis
            freeReplyObject(reply);
            reply = (redisReply*)redisCommand(context, "SET %s %s", redisQuery.c_str(), result.c_str());

            if (reply == nullptr) {
                std::cerr << "Redis SET command failed: " << context->errstr << std::endl;
                exit(1);
            }
        
            freeReplyObject(reply);
        }
    } else {
        std::cerr << "Unknown type: " << type << std::endl;
        exit(1);
    }
    //std::cout << result << '\n';
    _sockIO.writen(result.c_str(), result.size());
}

void TcpConnection::sendInLoop(const string &msg)
{
    if(_loop)
    {
        /* function<void()> f = std::bind(&TcpConnection::send, this, msg); */
        //void(function<void()> &&)
        _loop->runInLoop(std::bind(&TcpConnection::send, this, msg));
    }
}
string TcpConnection::receive()
{
    char buff[65535] = {0};
    //_sockIO.readLine(buff, sizeof(buff));
    _sockIO.readHttp(buff, sizeof(buff));
    return string(buff);
}

//查看连接是否断开
bool TcpConnection::isClosed() const
{
    char buf[10] = {0};
    int ret = ::recv(_sock.fd(), buf, sizeof(buf), MSG_PEEK);

    return (0 == ret);
}

string TcpConnection::toString()
{
    ostringstream oss;
    oss << _localAddr.ip() << ":"
        << _localAddr.port() << "---->"
        << _peerAddr.ip() << ":"
        << _peerAddr.port();

    return oss.str();
}

//获取本端的网络地址信息
InetAddress TcpConnection::getLocalAddr()
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(struct sockaddr );
    //获取本端地址的函数getsockname
    int ret = getsockname(_sock.fd(), (struct sockaddr *)&addr, &len);
    if(-1 == ret)
    {
        perror("getsockname");
    }

    return InetAddress(addr);
}

//获取对端的网络地址信息
InetAddress TcpConnection::getPeerAddr()
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(struct sockaddr );
    //获取对端地址的函数getpeername
    int ret = getpeername(_sock.fd(), (struct sockaddr *)&addr, &len);
    if(-1 == ret)
    {
        perror("getpeername");
    }

    return InetAddress(addr);
}
//添加三个回调的注册
void TcpConnection::setNewConnectionCallback(const TcpConnectionCallback &cb)
{
    _onNewConnection = cb;
}

void TcpConnection::setMessageCallback(const TcpConnectionCallback &cb)
{
    _onMessage = cb;
}

void TcpConnection::setCloseCallback(const TcpConnectionCallback &cb)
{
    _onClose = cb;
}

//添加三个回调的执行
void TcpConnection::handleNewConnectionCallback()
{
    if(_onNewConnection)
    {
        /* shared_ptr<TcpConnection>; */
        /* shared_ptr<TcpConnection> ccc(new TcpConnection()); */
        /* shared_ptr<TcpConnection> ccc(this); */
        /* _onNewConnection(shared_ptr<TcpConnection>(this)); */
        _onNewConnection(shared_from_this());
    }
    else
    {
        cout << "nullptr == _onNewConnection" << endl;
    }
}
void TcpConnection::handleMessageCallback()
{
    if(_onMessage)
    {
        _onMessage(shared_from_this());
    }
    else
    {
        cout << "nullptr == _onMessage" << endl;
    }
}

void TcpConnection::handleCloseCallback()
{
    if(_onClose)
    {
        _onClose(shared_from_this());
    }
    else
    {
        cout << "nullptr == _onClose" << endl;
    }
}
