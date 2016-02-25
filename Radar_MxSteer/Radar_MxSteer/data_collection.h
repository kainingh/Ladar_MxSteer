
#include "afxtempl.h"
#include<WinSock2.h>
#include<iostream>
#include<string>

using namespace std;

#pragma comment(lib, "WS2_32") 

/*
此头与data_collection.cpp 实现了与雷达的控制和数据传输
*/

#define IPaddress "192.168.0.10" //雷达IP地址
#define portNumber 10940 // 雷达端口号
class data_collection
{
private:
	SOCKET clientSocket;
public:
	bool InitWSA(WORD &wVersion, WSADATA *wsadata);  //开启WSA方法
	void cleanWSA();   //关闭WSA方法
	bool getLocalIP(char *IP);  
	void InitSockAddr(SOCKADDR_IN *pSockAddr, char *address = IPaddress, int netport = portNumber);    //用以确定协议，存放地址的方法
	SOCKET InitSocket();   // 初始化Socket方法
	bool bindAddr(SOCKADDR_IN *pSockAddr);   //绑定方法
	bool setListener(SOCKET * pSocket, int backlog);  //服务器端用以创建listener的方法
	bool setConnect(SOCKADDR_IN *serverAddr, int address_size);  // 客户端用以创建连接的方法
	void closeSocket();
};



