#ifndef CHATSERVICE_H   
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include "json.hpp"
#include "usermodel.hpp"
#include <mutex>
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmedel.hpp"
#include "redis.hpp"

using namespace std;
using namespace muduo;
using namespace muduo::net;
using json= nlohmann::json; 
// 表示处理消息的事件回调方法类型
using MsgHandler=std::function<void(const TcpConnectionPtr &conn,json &js,Timestamp)>;   //消息事件回调，std::fuction函数封装器。将三个数据信息封装成一个void类型的消息处理类。


//***************业务模块头文件**************

//聊天服务器业务类（单例模式实现）
class ChatService
{

public:
    // 获取单例对象的接口函数
    static ChatService *instance();

   // 处理登录业务
    void login(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 处理注册业务
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp time);

    //一对一聊天业务
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 添加好友业务
    void addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 创建群组业务
    void createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 加入群组业务
    void addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time);

     // 群组聊天业务
    void groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time);

    // 处理注销业务
    void loginout(const TcpConnectionPtr &conn, json &js, Timestamp time);

    //服务器异常，业务重置方法
    void reset();

    //处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);

    //获取消息处理对应的处理器
    MsgHandler getHandler(int msgid);

    // 从redis消息队列中获取订阅的消息
    void handleRedisSubscribeMessage(int, string);

private:
    ChatService();

    // 存储消息id和其对应的业务处理方法
    unordered_map<int,MsgHandler> _msgHandlerMap; //消息id处理表,

    // 存储在线用户的通信连接
    unordered_map<int, TcpConnectionPtr> _userConnMap;

    //定义互斥锁，保证_userConnMap的线程安全问题：因为回调时好多线程同步进行
    mutex _connMutex;

    //数据操作类对象
    UserModel _userModel;
    OfflineMsgModel _offlineMsgModel;
    FriendModel _friendModel;
    GroupModel _groupModel;

    // redis操作对象
    Redis _redis;

};






#endif