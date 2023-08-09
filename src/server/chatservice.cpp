#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>  //日志头文件
#include <vector>

using namespace std;
using namespace muduo;

// 获取单例对象的接口函数
ChatService *ChatService::instance()
{
    static ChatService service;
    return &service;
}


// 注册消息以及对应的Handler回调操作
ChatService::ChatService()
{
    // 用户基本业务管理相关事件处理回调注册
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});  //把业务模块login和数据类型LOGIN_MSG进行绑定
    //_msgHandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG,std::bind(&ChatService::addFriend, this, _1, _2, _3)});
     _msgHandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::loginout, this, _1, _2, _3)});

     //群组业务相关事件处理回调注册
    _msgHandlerMap.insert({CREATE_GROUP_MSG,std::bind(&ChatService::createGroup,this,_1,_2,_3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG,std::bind(&ChatService::addGroup,this,_1,_2,_3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG,std::bind(&ChatService::groupChat,this,_1,_2,_3)});

    // 连接redis服务器
    if (_redis.connect())
    {
        // 设置上报消息的回调
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, _1, _2));
    }
    
}


//服务器异常，业务重置方法
    void ChatService::reset()
    {
        //把online状态的用户，设置为offline
        _userModel.resetState();        //通过实例化数据层对象对数据层信息进行修改。

    }


//获取消息处理对应的处理器（获取相应的业务）
    MsgHandler ChatService::getHandler(int msgid){

        // 记录错误日志，msgid没有对应的事件处理回调
        auto it=_msgHandlerMap.find(msgid);
        if(it==_msgHandlerMap.end()){
            // 返回一个默认的处理器(匿名函数对象)，空操作
            return [=](const TcpConnectionPtr &conn, json &js, Timestamp) {
                LOG_ERROR << "msgid:" << msgid << " can not find handler!";
            };
        }
        else{
            return _msgHandlerMap[msgid];
        }
    }


// 处理登录业务  id  pwd   pwd
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    //LOG_INFO<<"do login service!!!";
    int id = js["id"].get<int>();  //js：默认都是字符串类型，需要转换类型
    string pwd=js["password"];

    User user =_userModel.query(id);
    if(user.getId()==id && user.getPwd()==pwd){     //查询成功

         if (user.getState() == "online")  //状态显示已登陆
        {
            // 该用户已经登录，不允许重复登录
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "this account is using, input another!";
            conn->send(response.dump());
        }
        else{

            // 登录成功，记录用户连接信息（长连接）
            {   //{}表示作用域，作用域结束进行解锁。

                lock_guard<mutex> lock(_connMutex);        //通过锁卫士lock_guard自动管理互斥锁的获取与释放。
                _userConnMap.insert({id, conn});           //存储通信连接，用于长连接。
            }
            

            // id用户登录成功后，向redis订阅channel(id)
            _redis.subscribe(id); 


            //登陆成功,更新用户状态信息，state offline——》online
            user.setState("online");
            _userModel.updataState(user);


            json response;   //返回消息
            response["msgid"]=LOGIN_MSG_ACK;
            response["errno"]=0;
            response["id"]=user.getId();
            response["name"]=user.getName();


            //查询该用户是否有离线消息
            vector<string> vec = _offlineMsgModel.query(id);
            if (!vec.empty())
            {
                response["offlinemsg"] = vec;   //容器可以直接与json进行读写。
                
                // 读取该用户的离线消息后，把该用户的所有离线消息删除掉
                _offlineMsgModel.remove(id);
            }
            

            // 查询该用户的好友信息并返回
            vector<User> userVec = _friendModel.query(id);
            if (!userVec.empty())
            {
                vector<string> vec2;
                for (User &user : userVec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }



            // 查询用户的群组信息
            vector<Group> groupuserVec = _groupModel.queryGroups(id);
            if (!groupuserVec.empty())
            {
                // group:[{groupid:[xxx, xxx, xxx, xxx]}]
                vector<string> groupV;       //id用户的群组消息（群id、群名、群成员等，有多个）
                for (Group &group : groupuserVec)
                {
                    json grpjson;
                    grpjson["id"] = group.getId();
                    grpjson["groupname"] = group.getName();
                    grpjson["groupdesc"] = group.getDesc();
                    vector<string> userV;
                    for (GroupUser &user : group.getUsers())
                    {
                        json js;
                        js["id"] = user.getId();
                        js["name"] = user.getName();
                        js["state"] = user.getState();
                        js["role"] = user.getRole();
                        userV.push_back(js.dump());
                    }
                    grpjson["users"] = userV;
                    groupV.push_back(grpjson.dump());
                }

                response["groups"] = groupV;
            }

            conn->send(response.dump());
        }
        
    }
    else{

        //用户不存在，用户存在但是密码错误，登录失败。
        json response;   //返回消息
        response["msgid"]=LOGIN_MSG_ACK;
        response["errno"]=1;
        response["errmsg"]="用户名或者密码错误";
        conn->send(response.dump());  //发送数据，序列化成字符串。
    }

}


// 处理注册业务  name  password
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    //LOG_INFO<<"do reg service!!!";
    string name =js["name"];
    string pwd=js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state=_userModel.insert(user);   //把数据传给数据层。业务模块操作对象，——userMedel数据模块操作参数。
    if(state){
        //注册成功
        json response;   //返回消息
        response["msgid"]=REG_MSG_ACK;
        response["errno"]=0;
        response["id"]=user.getId();
        conn->send(response.dump());
    }
    else{
        //注册失败
        json response;   //返回消息
        response["msgid"]=REG_MSG_ACK;
        response["errno"]=1;
        conn->send(response.dump());  //发送数据，序列化成字符串。
    }


}



 //处理客户端异常退出(连接还存在的情况)
    void ChatService::clientCloseException(const TcpConnectionPtr &conn){  //conn是一个智能指针

        User user;

        {
            lock_guard<mutex> lock(_connMutex);
            for(auto it=_userConnMap.begin();it!=_userConnMap.end();++it){

                if(it->second==conn){

                    //从map表中删除用户的链接信息
                    user.setId(it->first);
                    _userConnMap.erase(it);
                    break;
                }
            }
        }

        // 用户注销，相当于就是下线，在redis中取消订阅通道
        _redis.unsubscribe(user.getId()); 

        //更新用户状态信息
        if(user.getId()!=-1){  //找到该用户

             user.setState("offline");
            _userModel.updataState(user);
        }
       
    }


//一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int toid =js["toid"].get<int>();   //找到要发送到的用户

    {
        lock_guard<mutex> lock(_connMutex); 
        auto it=_userConnMap.find(toid);   //——userConnMap存储在线通信连接信息
        if(it!=_userConnMap.end())
        {
            //toid在线，转发消息，服务器主动推送消息给toid用户
            it->second->send(js.dump());
            return;
        }
    }

    // 查询toid是否在线(不在一台电脑上) 
    User user = _userModel.query(toid);
    if (user.getState() == "online")
    {
        _redis.publish(toid, js.dump());
        return;
    }

    //toid不在线：存放离线消息。
    _offlineMsgModel.insert(toid,js.dump()); 
    
}


// 添加好友业务 msgid id friendid
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    // 存储好友信息
    _friendModel.insert(userid, friendid);
}


// 处理注销业务(跟异常退出功能一样，置为离线状态，删除连接)
void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();

    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if (it != _userConnMap.end())
        {
            _userConnMap.erase(it);
        }
    }

    // 用户注销，相当于就是下线，在redis中取消订阅通道
    _redis.unsubscribe(userid); 

    // 更新用户的状态信息
    User user(userid, "", "", "offline");
    _userModel.updataState(user);
}



// 创建群组业务
    void ChatService::createGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
    {
        int userid = js["id"].get<int>();        //此人创建的群组，将该人加入群组并置为管理员
        string name = js["groupname"];
        string desc = js["groupdesc"];

        // 存储新创建的群组信息
        Group group(-1, name, desc);
        if (_groupModel.createGroup(group))      //createGroup：用引用的方式将群id传回group中
        {
            // 存储群组创建人信息
            _groupModel.addGroup(userid, group.getId(), "creator");      //创建一个群就相应的把创建的人加入其中
        }
    }



// 加入群组业务
void ChatService::addGroup(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "normal");
}



// 群组聊天业务
void ChatService::groupChat(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridVec = _groupModel.queryGroupUsers(userid, groupid);

    lock_guard<mutex> lock(_connMutex);
    for (int id : useridVec)
    {
        auto it = _userConnMap.find(id);
        if (it != _userConnMap.end())
        {
            // 转发群消息
            it->second->send(js.dump());
        }
        else
        {
            // 查询toid是否在线 
            User user = _userModel.query(id);
            if (user.getState() == "online")
            {
                _redis.publish(id, js.dump());
            }
            else
            {
                // 存储离线群消息
                _offlineMsgModel.insert(id, js.dump());
            }
        }
    }
}


// 从redis消息队列中获取订阅的消息
void ChatService::handleRedisSubscribeMessage(int userid, string msg)
{
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if (it != _userConnMap.end())
    {
        it->second->send(msg);
        return;
    }

    // 存储该用户的离线消息（如果对方正好下线,直接离线存储，不同服务器也可）
    _offlineMsgModel.insert(userid, msg);
}