#ifndef USERMODEL_H
#define USERMODEL_H


#include "user.hpp"

//User表的数据操作类:操作mysql表的
class UserModel{

public:
    //User表的添加操作
    bool insert(User &user);   //数据层打包好user对象，等待业务层传进来

    //根据用户号码查询用户信息
    User query(int id);

    //更新用户状态信息
    bool updataState(User user);

    //重置用户状态信息
    void resetState();

};



#endif















