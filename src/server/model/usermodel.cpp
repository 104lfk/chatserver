#include "usermodel.hpp"
#include "db.h"
#include <muduo/base/Logging.h>
#include <iostream>

using namespace std;

bool UserModel::insert(User &user){
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name, password, state) values('%s', '%s', '%s')",      //拼接完整的sql语句
            user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            // 获取插入成功的用户数据生成的主键id
            user.setId(mysql_insert_id(mysql.getConnection()));   //mysql.getConnection()):获取连接对象，mysql_insert_id：找到插入对象的主键，主键是自动生成的，外键都是传入的。
            return true;
        }
    }

    return false;
    
}

//根据用户号码查询用户信息
    User UserModel::query(int id){
        // 1.组装sql语句
        char sql[1024] = {0};
        sprintf(sql, "select * from user where id = %d ",      //拼接完整的sql语句
                id);

        MySQL mysql;
        if (mysql.connect())
        {
            MYSQL_RES *res = mysql.query(sql);   //查的时候查到的都是字符串
            if(res!=nullptr){
                MYSQL_ROW row = mysql_fetch_row(res);  //mysql_fetch_row(res)：获取结果集res的下一行，MYSQL_ROW：结果集中的一行数据（包括多个字段的字符串数组），
                if(row!=nullptr){

                    User user;
                    user.setId(atoi(row[0]));   //atoi:转整数。row[0]:字符串
                    user.setName(row[1]);
                    user.setPwd(row[2]);
                    user.setState(row[3]);
                    mysql_free_result(res);    //释放res指针的资源
                    return user;
                }
            }
        
        }

        return User();  //若没有找到，返回默认构造
    }


    //更新用户状态信息
    bool UserModel::updataState(User user){
        // 1.组装sql语句
        char sql[1024] = {0};
        sprintf(sql, "update user set state = '%s' where id = %d", user.getState().c_str(), user.getId());

        MySQL mysql;
        if (mysql.connect())
        {
            if (mysql.update(sql))
            {
                return true;
            }
        }
        return false; 

    }


    //重置用户状态信息
    void UserModel::resetState()
    {
        // 1.组装sql语句
    char sql[1024] = "update user set state = 'offline' where state = 'online'";

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
    
    }