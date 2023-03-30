# git使用

1. 克隆项目到本地
   
    ``` bash
    git clone https://github.com/wangsiyi/net-pan.git
    ```

2. 查看状态

    ``` bash
    git status
    ```

3. 提交修改到暂存区(提交前需要先查看状态，有没有非代码文件，如果有，需要加入到.gitignore文件中)

    ``` bash
    git add test.c
    ```

4. 将暂存区内容提交到仓库

    ``` bash
    git commit -m "commit message"
    ```

5. 查看提交历史信息

    ``` bash
    git log
    ```

6. 将本地仓库提交到远程

    ``` bash
    git push
    ```

7. 如果与远程仓库的提交历史不一致，需要先pull。之后查看状态，如果有冲突，解决冲突后再 add -> commit -> push

    ``` bash
    git pull
    ```

# 目录结构

工程目录包含以下文件：

1. src 用于存放.c文件
2. inc 用于存放头文件.h文件
3. config 用于存放配置文件
4. bin 用于存放生成的可执行程序
5. obj 用于存放编译过程中生成的其他文件
6. makefile make文件
7. readme.md 使用文档



``` C
#ifdef MY_MAIN

#include ...

int main(int argc, char **argv) {
	...
}

#endif
```



# 接口定义

## 程序编写和命名规范

1. 所有函数名和变量名使用驼峰命名，同时不能写简称，如：`myFunction`, `myValue`
2. 指针需要在变量名前面加上`p`，如：`pMyValue`，`pMyFunction`
3. 所有函数返回值为`int`类型，返回0表示成功，其他值表示失败
4. 所有函数的传入参数放在传入传出参数前面
5. 所有函数必须有相应注释，注释格式如下：
	``` C
	/*
	 * @brief 描述函数的功能
	 * @param in a 传入参数
	 * @param in b 传入参数
	 * @param out sum 传出参数
	 * @return 0表示执行成功，其他值需要说明
	 */
	```

## 接口

``` C
int changeDirectory(const char *path);
int listFile(const char *path, char *message, int length);
int printWorkingDirectory(char *message, int length);
int removeFile(const char *fileName);
int makeDirectory(const char *path);
int getFile(int clientFd, const char *fileName);
int putFile(int clientFd, const char *fileName);
int passwordConfirm(const char *userName, const char *password);
```

## 断点续传实现

在接收文件时，通过`ftruncate`函数设置文件大小，如果接收文件失败，则通过`ftruncate`将文件大小设成已接收的大小。

之后重传时就可以提供已传输大小，让服务端或者客户端偏移到指定位置后重新传输

# 通信协议

``` C
enum {
    CD = 0x1000,
    LS = 0x1001,
    PWD = 0x1002,
    RM = 0x1003,
    MKDIR = 0x1004,
    PUTS = 0x1005,
    GETS = 0x1006,
    MESSAGE = 0x1007,  // 服务器回传给客户端的显示信息

    FILE_SIZE = 0x2001,  // 传输文件的大小
    FILE_DATA = 0x2002,  // 实际传输的文件内容
    FILE_MD5 = 0x2003,  // 文件的MD5值
    FILE_OFFSET = 0x2004  // 文件偏移
};

typedef struct {
    int type;  // 传输的数据类型
    int size;  // data的大小
    char data[];  // 实际使用的信息
} protocol_t;
```

#### 客户端往服务端发送文件流程

1. 服务端接收到客户端上传文件请求后，直接调用函数`getFile()`，之后为函数中的流程
2. 查看请求的文件名是否已经在服务器上存在，如果存在回消息给客户端，同名文件已存在请删除后重新上传，否则继续执行
3. 接收客户端上传的MD5
4. 接收客户端上传的文件大小
5. 开始正常传输文件

#### 服务端往客户端发送文件流程

1. 服务端接收到客户端下载文件请求后，直接调用函数`putFile()`，之后为函数流程
2. 接收客户端发送的偏移量
3. 给客户端发送文件MD5
4. 发送文件大小给客户端
5. 开始正常传输文件

客户端在发送文件名后，查看本地是否已经存在该文件，如果存在，传输偏移量给服务端，如果不存在，传输偏移量为0