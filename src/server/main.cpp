#include "chatserver.hpp"
#include "signal.h"
#include "chatservice.hpp"
#include <iostream>
using namespace std;


//处理服务器ctrl+c结束后，重置user的状态信息
void resetHandler(int){                   //接受整形数据：信号的编号
    ChatService::instance()->reset();     //网络层调用业务层重置业务
    exit(0);
}


int main(int argc, char **argv){

    signal(SIGINT,resetHandler);   //signal:注册信号处理函数，SIGINT：中断信号，resetHandler：处理中断的函数。

    if (argc < 3)
    {
        cerr << "command invalid! example: ./ChatServer 127.0.0.1 6000" << endl;
        exit(-1);
    }

    // 解析通过命令行参数传递的ip和port
    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);


    EventLoop loop;
    InetAddress addr(ip,port);
    ChatServer server(&loop, addr, "ChatServer");

    server.start();
    loop.loop();
    return 0;
}