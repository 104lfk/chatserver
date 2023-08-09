/*
代码功能：muduo网络库的编程很容易，要实现基于muduo网络库的服务器和客户端程序，只需要简单的组合
TcpServer和TcpClient就可以，代码实现如下：

muduo网络库给用户提供了两个主要类
    TCPServer：用于编写服务器程序
    TCPClient：用于编写客户端程序

    epoll+线性池
    作用：将网络I/O代码（不关注）和业务代码区分开来（包括用户的连接与断开、用户的读写事件）
*/



#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <functional> 
#include <string>
using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace placeholders;


/*基于muduo网络库开发服务器程序
1.组合TcpServer对象
2.创建EventLoop事件循环对象的指针
3.明确TcpServer构造函数需要什么参数，输出ChatServer的构造函数
4.在当前服务器类的构造函数当中，注册处理连接的回调函数和处理读写时间的回调函数
5.设置合适的服务端线程数量，muduo库会自己分配I/O线程和worker线程
*/

class ChatServer{
    public:
        //1、初始化服务器有参构造函数
        ChatServer(EventLoop* loop,     //事件循环
            const InetAddress& listenAddr,  //IP和端口
            const string& nameArg)         //服务器的名字
            :_server(loop,listenAddr,nameArg),_loop(loop){  //成员初始化列表的方式给变量赋值

                //给服务器注册用户连接的创建和断开回调，回调（连接的时机由网络库确定，我们只需要写怎么做即可，网络库确定一个事件时回调我们写的业务代码-与上面的epoll和线程池相对应）,新的用户连接或者用户断开连接时，该回调函数将被调用
                _server.setConnectionCallback(std::bind(&ChatServer::onConnect,this,_1));   //onConnect:有一个this对象，一个参数绑定到一起
/*
下面代码功能：将当前的ChatServer对象和onMessage函数绑定在一起，并将绑定后的可调用对象作为消息处理回调函数，用于处理客户端发送的消息。
当有消息到达时，服务器会自动调用该回调函数，并传递Tcp连接对象、缓冲区指针和时间戳等参数给onMessage函数进行处理。
*/
                // 给服务器注册用户读写事件回调
                _server.setMessageCallback(std::bind(&ChatServer::onMessage,this,_1,_2,_3));
                
                //设置合适服务端线程数量,一个I/O线程，三个work线程
                _server.setThreadNum(4);
            }

            //2、开启事件循环
            void start(){
                _server.start();
            }

    private:
        //专门处理用户的连接创建和断开
        void onConnect(const TcpConnectionPtr &conn)
        {
            if(conn->connected()){
                cout<<conn->peerAddress().toIpPort()<<"->"<<conn->localAddress().toIpPort()<<"state:online"<<endl;
            }
            else{
                cout<<conn->peerAddress().toIpPort()<<"->"<<conn->localAddress().toIpPort()<<"state:offline"<<endl;
                conn->shutdown();
            }
        }

        // 专门处理用户的读写事件
        void onMessage(const TcpConnectionPtr &conn, // 连接
                            Buffer *buffer,          //缓冲区
                            Timestamp time)          //接收到数据的时间
                            {
                                string buf=buffer->retrieveAllAsString();  //将缓冲区数据作为字符串返回
                                cout << "recv data:" << buf << " time:" << time.toFormattedString() << endl; //打印数据
                                conn->send(buf);  //将缓冲区数据重新发回去
                            }

        TcpServer _server;  //1
        EventLoop *_loop;    //2 epoll
};

int main()
{
    EventLoop loop; // epoll
    InetAddress addr("127.0.0.1", 6000);
    ChatServer server(&loop, addr, "ChatServer");

    server.start(); // listenfd epoll_ctl=>epoll
    loop.loop();    // epoll_wait以阻塞方式等待新用户连接，已连接用户的读写事件等

    return 0;
}