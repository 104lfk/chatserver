#include "json.hpp"
using json = nlohmann::json;

#include<iostream>
#include<vector>
#include<map>
#include<string>
using namespace std;


//json序列化示例1:序列化（转字符串）普通数据
void fun1(){
json js;
// 添加数组
js["id"] = {1,2,3,4,5};
// 添加key-value
js["name"] = "zhang san";
// 添加对象
js["msg"]["zhang san"] = "hello world";
js["msg"]["liu shuo"] = "hello china";
// 上面等同于下面这句一次性添加数组对象
js["msg"] = {{"zhang san", "hello world"}, {"liu shuo", "hello china"}};

string sendBuf=js.dump();  //dump（）：将json序列化转成字符串格式。
cout<<sendBuf.c_str()<< endl;
//cout<<js<<endl;
 }


//json序列化示例2：序列化容器
void fun2(){

    json js;

// 直接序列化一个vector容器
vector<int> vec;
vec.push_back(1);
vec.push_back(2);
vec.push_back(5);
js["list"] = vec;

// 直接序列化一个map容器
map<int, string> m;
m.insert({1, "黄山"});
m.insert({2, "华山"});
m.insert({3, "泰山"});
js["path"] = m;
cout<<js<<endl;
}


int main(){
fun2();
    return 0;
}