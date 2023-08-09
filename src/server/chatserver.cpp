#include "chatserver.hpp"
#include <functional>  //提供bind绑定函数
#include "json.hpp"
#include <string>
#include "chatservice.hpp"

using namespace std;
using namespace placeholders;  //提供占位符
using json= nlohmann::json;    //简化代码，不用写作用域了


// 初始化聊天服务器对象
ChatServer::ChatServer(EventLoop *loop,
                       const InetAddress &listenAddr,
                       const string &nameArg)
    : _server(loop, listenAddr, nameArg), _loop(loop)
{
    // 注册链接回调
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));

    // 注册消息回调
    _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));

    // 设置线程数量
    _server.setThreadNum(4);
}


// 启动服务
void ChatServer::start()
{
    _server.start();
}


// 上报链接相关信息的回调函数
void ChatServer::onConnection(const TcpConnectionPtr &conn)
{
    // 客户端断开链接
    if (!conn->connected())
    {
        ChatService::instance()->clientCloseException(conn);   //通过业务层的单例对象处理异常关闭业务。
        conn->shutdown();
    }
}

// 上报读写事件相关信息的回调函数
void ChatServer::onMessage(const TcpConnectionPtr &conn,
                           Buffer *buffer,
                           Timestamp time)
{
    string buf = buffer->retrieveAllAsString();

    // // 测试，添加json打印代码
    // cout << buf << endl;

    // // 数据的反序列化
    json js = json::parse(buf);
    
    // // 达到的目的：完全解耦网络模块的代码和业务模块的代码
    // // 通过js["msgid"] 获取=》业务handler=》conn  js  time
    auto msgHandler=ChatService::instance()->getHandler(js["msgid"].get<int>());
    // // 回调消息绑定好的事件处理器，来执行相应的业务处理
     msgHandler(conn, js, time);
}