#include "db.h"
#include <muduo/base/Logging.h>  //日志头文件


// 数据库配置信息
static string server = "127.0.0.1";
static string user = "root";
static string password = "123456";
static string dbname = "chat";


// 初始化数据库连接
MySQL::MySQL()
{
    _conn = mysql_init(nullptr);  //开辟数据库资源空间
}

// 释放数据库连接资源
    MySQL::~MySQL()
{
    if (_conn != nullptr)
    mysql_close(_conn);     //释放空间
}

// 连接数据库
    bool MySQL::connect()
{
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(),      //将字符串（C++风格）转化为常量指针的形式（c风格），连接成功时返回一个连接对象的mysql指针
    password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    if (p != nullptr)
    {
        //C++代码默认的编码字符是ASCII，如果不设置，从MySql上拉下来的中文显示乱码
        mysql_query(_conn, "set names gbk");     //该函数向数据库服务器发送SQL查询或更新语句。
        LOG_INFO << "connect mysql success!";
    }
    else{
        LOG_INFO << "connect mysql fail!";
    }
    return p;
}

// 更新操作
    bool MySQL::update(string sql)   //string sql为sql语句。
{
    if (mysql_query(_conn, sql.c_str()))   //更新查询为0代表成功
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
        << sql << "更新失败!";
        return false;
        }
    return true;
}

// 查询操作
    MYSQL_RES* MySQL::query(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
        << sql << "查询失败!";
        return nullptr;
    }
    return mysql_use_result(_conn);   //获取查询结果集。
}


// 获取连接
    MYSQL* MySQL::getConnection(){
        return _conn;
    }

