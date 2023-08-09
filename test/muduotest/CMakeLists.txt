cmake_minimum_required(VERSION 3.0)
project(chat)

# 配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)   #生成可执行文件时包含调试信息

# 配置最终的可执行文件输出的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

#aux_source_directory(. SRC_LIST)  #将当前目录下的所有源文件放入变量SRC中
set( SRC_LIST  #将main、test源文件放入变量中
./muduo_server.cpp)
add_executable(server ${SRC_LIST})  #生成可执行文件

target_link_libraries(server muduo_net muduo_base pthread)   #server这个目标程序需要连接这三个库文件

#添加头文件路径，相当于makefile里面的-I
#include_directories(${PROJECT_SOURCE_DIR}/include)
#link_directories (${PROJECT_SOURCE_DIR}/lib) # 库文件搜索路径


#执行cmake ..命令（找CMakeLists进行构建），然后执行make。若执行cmake.会将编译文件生成到当前文件夹下（一般在build中执行这两个操作）。

# 加载子目录
#add_subdirectory(muduotest) #在子目录中找CMakeLists进行编译